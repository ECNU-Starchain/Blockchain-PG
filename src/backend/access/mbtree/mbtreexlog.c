/*-------------------------------------------------------------------------
 *
 * nmbtxlog.c
 *	  WAL replay logic for mbtrees.
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/access/nmbtree/nmbtxlog.c,v 1.55 2009/06/11 14:48:54 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/mbtree.h"
#include "access/transam.h"
#include "storage/bufmgr.h"

/*
 * We must keep track of expected insertions due to page splits, and apply
 * them manually if they are not seen in the WAL log during replay.  This
 * makes it safe for page insertion to be a multiple-WAL-action process.
 *
 * Similarly, deletion of an only child page and deletion of its parent page
 * form multiple WAL log entries, and we have to be prepared to follow through
 * with the deletion if the log ends between.
 *
 * The data structure is a simple linked list --- this should be good enough,
 * since we don't expect a page split or multi deletion to remain incomplete
 * for long.  In any case we need to respect the order of operations.
 */
typedef struct mbt_incomplete_action
{
	RelFileNode node;			/* the index */
	bool		is_split;		/* T = pending split, F = pending delete */
	/* these fields are for a split: */
	bool		is_root;		/* we split the root */
	BlockNumber leftblk;		/* left half of split */
	BlockNumber rightblk;		/* right half of split */
	/* these fields are for a delete: */
	BlockNumber delblk;			/* parent block to be deleted */
} mbt_incomplete_action;

static List *incomplete_actions;


static void
log_incomplete_split(RelFileNode node, BlockNumber leftblk,
					 BlockNumber rightblk, bool is_root)
{
	mbt_incomplete_action *action = palloc(sizeof(mbt_incomplete_action));

	action->node = node;
	action->is_split = true;
	action->is_root = is_root;
	action->leftblk = leftblk;
	action->rightblk = rightblk;
	incomplete_actions = lappend(incomplete_actions, action);
}

static void
forget_matching_split(RelFileNode node, BlockNumber downlink, bool is_root)
{
	ListCell   *l;

	foreach(l, incomplete_actions)
	{
		mbt_incomplete_action *action = (mbt_incomplete_action *) lfirst(l);

		if (RelFileNodeEquals(node, action->node) &&
			action->is_split &&
			downlink == action->rightblk)
		{
			if (is_root != action->is_root)
				elog(LOG, "forget_matching_split: fishy is_root data (expected %d, got %d)",
					 action->is_root, is_root);
			incomplete_actions = list_delete_ptr(incomplete_actions, action);
			pfree(action);
			break;				/* need not look further */
		}
	}
}

static void
log_incomplete_deletion(RelFileNode node, BlockNumber delblk)
{
	mbt_incomplete_action *action = palloc(sizeof(mbt_incomplete_action));

	action->node = node;
	action->is_split = false;
	action->delblk = delblk;
	incomplete_actions = lappend(incomplete_actions, action);
}

static void
forget_matching_deletion(RelFileNode node, BlockNumber delblk)
{
	ListCell   *l;

	foreach(l, incomplete_actions)
	{
		mbt_incomplete_action *action = (mbt_incomplete_action *) lfirst(l);

		if (RelFileNodeEquals(node, action->node) &&
			!action->is_split &&
			delblk == action->delblk)
		{
			incomplete_actions = list_delete_ptr(incomplete_actions, action);
			pfree(action);
			break;				/* need not look further */
		}
	}
}

/*
 * _mbt_restore_page -- re-enter all the index tuples on a page
 *
 * The page is freshly init'd, and *from (length len) is a copy of what
 * had been its upper part (pd_upper to pd_special).  We assume that the
 * tuples had been added to the page in item-number order, and therefore
 * the one with highest item number appears first (lowest on the page).
 *
 * NOTE: the way this routine is coded, the rebuilt page will have the items
 * in correct itemno sequence, but physically the opposite order from the
 * original, because we insert them in the opposite of itemno order.  This
 * does not matter in any current mbtree code, but it's something to keep an
 * eye on.	Is it worth changing just on general principles?  See also the
 * notes in mbtree_xlog_split().
 */
static void
_mbt_restore_page(Page page, char *from, int len)
{
	IndexTupleData itupdata;
	Size		itemsz;
	char	   *end = from + len;

	for (; from < end;)
	{
		/* Need to copy tuple header due to alignment considerations */
		memcpy(&itupdata, from, sizeof(IndexTupleData));
		itemsz = IndexTupleDSize(itupdata);
		itemsz = MAXALIGN(itemsz);
		if (PageAddItem(page, (Item) from, itemsz, FirstOffsetNumber,
						false, false) == InvalidOffsetNumber)
			elog(PANIC, "_mbt_restore_page: cannot add item to page");
		from += itemsz;
	}
}

static void
_mbt_restore_meta(RelFileNode rnode, XLogRecPtr lsn,
				 BlockNumber root, uint32 level,
				 BlockNumber fastroot, uint32 fastlevel)
{
	Buffer		metabuf;
	Page		metapg;
	mbtMetaPageData *md;
	mbtPageOpaque pageop;

	metabuf = XLogReadBuffer(rnode, mbtREE_METAPAGE, true);
	Assert(BufferIsValid(metabuf));
	metapg = BufferGetPage(metabuf);

	_mbt_pageinit(metapg, BufferGetPageSize(metabuf));

	md = mbtPageGetMeta(metapg);
	md->mbtm_magic = mbtREE_MAGIC;
	md->mbtm_version = mbtREE_VERSION;
	md->mbtm_root = root;
	md->mbtm_level = level;
	md->mbtm_fastroot = fastroot;
	md->mbtm_fastlevel = fastlevel;

	pageop = (mbtPageOpaque) PageGetSpecialPointer(metapg);
	pageop->mbtpo_flags = mbtP_META;

	/*
	 * Set pd_lower just past the end of the metadata.	This is not essential
	 * but it makes the page look compressible to xlog.c.
	 */
	((PageHeader) metapg)->pd_lower =
		((char *) md + sizeof(mbtMetaPageData)) - (char *) metapg;

	PageSetLSN(metapg, lsn);
	PageSetTLI(metapg, ThisTimeLineID);
	MarkBufferDirty(metabuf);
	UnlockReleaseBuffer(metabuf);
}

static void
mbtree_xlog_insert(bool isleaf, bool ismeta,
				  XLogRecPtr lsn, XLogRecord *record)
{
	xl_mbtree_insert *xlrec = (xl_mbtree_insert *) XLogRecGetData(record);
	Buffer		buffer;
	Page		page;
	char	   *datapos;
	int			datalen;
	xl_mbtree_metadata md;
	BlockNumber downlink = 0;

	datapos = (char *) xlrec + SizeOfmbtreeInsert;
	datalen = record->xl_len - SizeOfmbtreeInsert;
	if (!isleaf)
	{
		memcpy(&downlink, datapos, sizeof(BlockNumber));
		datapos += sizeof(BlockNumber);
		datalen -= sizeof(BlockNumber);
	}
	if (ismeta)
	{
		memcpy(&md, datapos, sizeof(xl_mbtree_metadata));
		datapos += sizeof(xl_mbtree_metadata);
		datalen -= sizeof(xl_mbtree_metadata);
	}

	if ((record->xl_info & XLR_BKP_BLOCK_1) && !ismeta && isleaf)
		return;					/* nothing to do */

	if (!(record->xl_info & XLR_BKP_BLOCK_1))
	{
		buffer = XLogReadBuffer(xlrec->target.node,
							 ItemPointerGetBlockNumber(&(xlrec->target.tid)),
								false);
		if (BufferIsValid(buffer))
		{
			page = (Page) BufferGetPage(buffer);

			if (XLByteLE(lsn, PageGetLSN(page)))
			{
				UnlockReleaseBuffer(buffer);
			}
			else
			{
				if (PageAddItem(page, (Item) datapos, datalen,
							ItemPointerGetOffsetNumber(&(xlrec->target.tid)),
								false, false) == InvalidOffsetNumber)
					elog(PANIC, "mbtree_insert_redo: failed to add item");

				PageSetLSN(page, lsn);
				PageSetTLI(page, ThisTimeLineID);
				MarkBufferDirty(buffer);
				UnlockReleaseBuffer(buffer);
			}
		}
	}

	if (ismeta)
		_mbt_restore_meta(xlrec->target.node, lsn,
						 md.root, md.level,
						 md.fastroot, md.fastlevel);

	/* Forget any split this insertion completes */
	if (!isleaf)
		forget_matching_split(xlrec->target.node, downlink, false);
}

static void
mbtree_xlog_split(bool onleft, bool isroot,
				 XLogRecPtr lsn, XLogRecord *record)
{
	xl_mbtree_split *xlrec = (xl_mbtree_split *) XLogRecGetData(record);
	Buffer		rbuf;
	Page		rpage;
	mbtPageOpaque ropaque;
	char	   *datapos;
	int			datalen;
	OffsetNumber newitemoff = 0;
	Item		newitem = NULL;
	Size		newitemsz = 0;
	Item		left_hikey = NULL;
	Size		left_hikeysz = 0;

	datapos = (char *) xlrec + SizeOfmbtreeSplit;
	datalen = record->xl_len - SizeOfmbtreeSplit;

	/* Forget any split this insertion completes */
	if (xlrec->level > 0)
	{
		/* we assume SizeOfmbtreeSplit is at least 16-bit aligned */
		BlockNumber downlink = BlockIdGetBlockNumber((BlockId) datapos);

		datapos += sizeof(BlockIdData);
		datalen -= sizeof(BlockIdData);

		forget_matching_split(xlrec->node, downlink, false);

		/* Extract left hikey and its size (still assuming 16-bit alignment) */
		if (!(record->xl_info & XLR_BKP_BLOCK_1))
		{
			/* We assume 16-bit alignment is enough for IndexTupleSize */
			left_hikey = (Item) datapos;
			left_hikeysz = MAXALIGN(IndexTupleSize(left_hikey));

			datapos += left_hikeysz;
			datalen -= left_hikeysz;
		}
	}

	/* Extract newitem and newitemoff, if present */
	if (onleft)
	{
		/* Extract the offset (still assuming 16-bit alignment) */
		memcpy(&newitemoff, datapos, sizeof(OffsetNumber));
		datapos += sizeof(OffsetNumber);
		datalen -= sizeof(OffsetNumber);
	}

	if (onleft && !(record->xl_info & XLR_BKP_BLOCK_1))
	{
		/*
		 * We assume that 16-bit alignment is enough to apply IndexTupleSize
		 * (since it's fetching from a uint16 field) and also enough for
		 * PageAddItem to insert the tuple.
		 */
		newitem = (Item) datapos;
		newitemsz = MAXALIGN(IndexTupleSize(newitem));
		datapos += newitemsz;
		datalen -= newitemsz;
	}

	/* Reconstruct right (new) sibling from scratch */
	rbuf = XLogReadBuffer(xlrec->node, xlrec->rightsib, true);
	Assert(BufferIsValid(rbuf));
	rpage = (Page) BufferGetPage(rbuf);

	_mbt_pageinit(rpage, BufferGetPageSize(rbuf));
	ropaque = (mbtPageOpaque) PageGetSpecialPointer(rpage);

	ropaque->mbtpo_prev = xlrec->leftsib;
	ropaque->mbtpo_next = xlrec->rnext;
	ropaque->mbtpo.level = xlrec->level;
	ropaque->mbtpo_flags = (xlrec->level == 0) ? mbtP_LEAF : 0;
	ropaque->mbtpo_cycleid = 0;

	_mbt_restore_page(rpage, datapos, datalen);

	/*
	 * On leaf level, the high key of the left page is equal to the first key
	 * on the right page.
	 */
	if (xlrec->level == 0)
	{
		ItemId		hiItemId = PageGetItemId(rpage, P_FIRSTDATAKEY(ropaque));

		left_hikey = PageGetItem(rpage, hiItemId);
		left_hikeysz = ItemIdGetLength(hiItemId);
	}

	PageSetLSN(rpage, lsn);
	PageSetTLI(rpage, ThisTimeLineID);
	MarkBufferDirty(rbuf);

	/* don't release the buffer yet; we touch right page's first item below */

	/*
	 * Reconstruct left (original) sibling if needed.  Note that this code
	 * ensures that the items remaining on the left page are in the correct
	 * item number order, but it does not reproduce the physical order they
	 * would have had.	Is this worth changing?  See also _mbt_restore_page().
	 */
	if (!(record->xl_info & XLR_BKP_BLOCK_1))
	{
		Buffer		lbuf = XLogReadBuffer(xlrec->node, xlrec->leftsib, false);

		if (BufferIsValid(lbuf))
		{
			Page		lpage = (Page) BufferGetPage(lbuf);
			mbtPageOpaque lopaque = (mbtPageOpaque) PageGetSpecialPointer(lpage);

			if (!XLByteLE(lsn, PageGetLSN(lpage)))
			{
				OffsetNumber off;
				OffsetNumber maxoff = PageGetMaxOffsetNumber(lpage);
				OffsetNumber deletable[MaxOffsetNumber];
				int			ndeletable = 0;

				/*
				 * Remove the items from the left page that were copied to the
				 * right page.	Also remove the old high key, if any. (We must
				 * remove everything before trying to insert any items, else
				 * we risk not having enough space.)
				 */
				if (!P_RIGHTMOST(lopaque))
				{
					deletable[ndeletable++] = P_HIKEY;

					/*
					 * newitemoff is given to us relative to the original
					 * page's item numbering, so adjust it for this deletion.
					 */
					newitemoff--;
				}
				for (off = xlrec->firstright; off <= maxoff; off++)
					deletable[ndeletable++] = off;
				if (ndeletable > 0)
					PageIndexMultiDelete(lpage, deletable, ndeletable);

				/*
				 * Add the new item if it was inserted on left page.
				 */
				if (onleft)
				{
					if (PageAddItem(lpage, newitem, newitemsz, newitemoff,
									false, false) == InvalidOffsetNumber)
						elog(PANIC, "failed to add new item to left page after split");
				}

				/* Set high key */
				if (PageAddItem(lpage, left_hikey, left_hikeysz,
								P_HIKEY, false, false) == InvalidOffsetNumber)
					elog(PANIC, "failed to add high key to left page after split");

				/* Fix opaque fields */
				lopaque->mbtpo_flags = (xlrec->level == 0) ? mbtP_LEAF : 0;
				lopaque->mbtpo_next = xlrec->rightsib;
				lopaque->mbtpo_cycleid = 0;

				PageSetLSN(lpage, lsn);
				PageSetTLI(lpage, ThisTimeLineID);
				MarkBufferDirty(lbuf);
			}

			UnlockReleaseBuffer(lbuf);
		}
	}

	/* We no longer need the right buffer */
	UnlockReleaseBuffer(rbuf);

	/* Fix left-link of the page to the right of the new right sibling */
	if (xlrec->rnext != P_NONE && !(record->xl_info & XLR_BKP_BLOCK_2))
	{
		Buffer		buffer = XLogReadBuffer(xlrec->node, xlrec->rnext, false);

		if (BufferIsValid(buffer))
		{
			Page		page = (Page) BufferGetPage(buffer);

			if (!XLByteLE(lsn, PageGetLSN(page)))
			{
				mbtPageOpaque pageop = (mbtPageOpaque) PageGetSpecialPointer(page);

				pageop->mbtpo_prev = xlrec->rightsib;

				PageSetLSN(page, lsn);
				PageSetTLI(page, ThisTimeLineID);
				MarkBufferDirty(buffer);
			}
			UnlockReleaseBuffer(buffer);
		}
	}

	/* The job ain't done till the parent link is inserted... */
	log_incomplete_split(xlrec->node,
						 xlrec->leftsib, xlrec->rightsib, isroot);
}

static void
mbtree_xlog_delete(XLogRecPtr lsn, XLogRecord *record)
{
	xl_mbtree_delete *xlrec;
	Buffer		buffer;
	Page		page;
	mbtPageOpaque opaque;

	if (record->xl_info & XLR_BKP_BLOCK_1)
		return;

	xlrec = (xl_mbtree_delete *) XLogRecGetData(record);
	buffer = XLogReadBuffer(xlrec->node, xlrec->block, false);
	if (!BufferIsValid(buffer))
		return;
	page = (Page) BufferGetPage(buffer);

	if (XLByteLE(lsn, PageGetLSN(page)))
	{
		UnlockReleaseBuffer(buffer);
		return;
	}

	if (record->xl_len > SizeOfmbtreeDelete)
	{
		OffsetNumber *unused;
		OffsetNumber *unend;

		unused = (OffsetNumber *) ((char *) xlrec + SizeOfmbtreeDelete);
		unend = (OffsetNumber *) ((char *) xlrec + record->xl_len);

		PageIndexMultiDelete(page, unused, unend - unused);
	}

	/*
	 * Mark the page as not containing any LP_DEAD items --- see comments in
	 * _mbt_delitems().
	 */
	opaque = (mbtPageOpaque) PageGetSpecialPointer(page);
	opaque->mbtpo_flags &= ~mbtP_HAS_GARBAGE;

	PageSetLSN(page, lsn);
	PageSetTLI(page, ThisTimeLineID);
	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);
}

static void
mbtree_xlog_delete_page(uint8 info, XLogRecPtr lsn, XLogRecord *record)
{
	xl_mbtree_delete_page *xlrec = (xl_mbtree_delete_page *) XLogRecGetData(record);
	BlockNumber parent;
	BlockNumber target;
	BlockNumber leftsib;
	BlockNumber rightsib;
	Buffer		buffer;
	Page		page;
	mbtPageOpaque pageop;

	parent = ItemPointerGetBlockNumber(&(xlrec->target.tid));
	target = xlrec->deadblk;
	leftsib = xlrec->leftblk;
	rightsib = xlrec->rightblk;

	/* parent page */
	if (!(record->xl_info & XLR_BKP_BLOCK_1))
	{
		buffer = XLogReadBuffer(xlrec->target.node, parent, false);
		if (BufferIsValid(buffer))
		{
			page = (Page) BufferGetPage(buffer);
			pageop = (mbtPageOpaque) PageGetSpecialPointer(page);
			if (XLByteLE(lsn, PageGetLSN(page)))
			{
				UnlockReleaseBuffer(buffer);
			}
			else
			{
				OffsetNumber poffset;

				poffset = ItemPointerGetOffsetNumber(&(xlrec->target.tid));
				if (poffset >= PageGetMaxOffsetNumber(page))
				{
					Assert(info == XLOG_mbtREE_DELETE_PAGE_HALF);
					Assert(poffset == P_FIRSTDATAKEY(pageop));
					PageIndexTupleDelete(page, poffset);
					pageop->mbtpo_flags |= mbtP_HALF_DEAD;
				}
				else
				{
					ItemId		itemid;
					IndexTuple	itup;
					OffsetNumber nextoffset;

					Assert(info != XLOG_mbtREE_DELETE_PAGE_HALF);
					itemid = PageGetItemId(page, poffset);
					itup = (IndexTuple) PageGetItem(page, itemid);
					ItemPointerSet(&(itup->t_tid), rightsib, P_HIKEY);
					nextoffset = OffsetNumberNext(poffset);
					PageIndexTupleDelete(page, nextoffset);
				}

				PageSetLSN(page, lsn);
				PageSetTLI(page, ThisTimeLineID);
				MarkBufferDirty(buffer);
				UnlockReleaseBuffer(buffer);
			}
		}
	}

	/* Fix left-link of right sibling */
	if (!(record->xl_info & XLR_BKP_BLOCK_2))
	{
		buffer = XLogReadBuffer(xlrec->target.node, rightsib, false);
		if (BufferIsValid(buffer))
		{
			page = (Page) BufferGetPage(buffer);
			if (XLByteLE(lsn, PageGetLSN(page)))
			{
				UnlockReleaseBuffer(buffer);
			}
			else
			{
				pageop = (mbtPageOpaque) PageGetSpecialPointer(page);
				pageop->mbtpo_prev = leftsib;

				PageSetLSN(page, lsn);
				PageSetTLI(page, ThisTimeLineID);
				MarkBufferDirty(buffer);
				UnlockReleaseBuffer(buffer);
			}
		}
	}

	/* Fix right-link of left sibling, if any */
	if (!(record->xl_info & XLR_BKP_BLOCK_3))
	{
		if (leftsib != P_NONE)
		{
			buffer = XLogReadBuffer(xlrec->target.node, leftsib, false);
			if (BufferIsValid(buffer))
			{
				page = (Page) BufferGetPage(buffer);
				if (XLByteLE(lsn, PageGetLSN(page)))
				{
					UnlockReleaseBuffer(buffer);
				}
				else
				{
					pageop = (mbtPageOpaque) PageGetSpecialPointer(page);
					pageop->mbtpo_next = rightsib;

					PageSetLSN(page, lsn);
					PageSetTLI(page, ThisTimeLineID);
					MarkBufferDirty(buffer);
					UnlockReleaseBuffer(buffer);
				}
			}
		}
	}

	/* Rewrite target page as embty deleted page */
	buffer = XLogReadBuffer(xlrec->target.node, target, true);
	Assert(BufferIsValid(buffer));
	page = (Page) BufferGetPage(buffer);

	_mbt_pageinit(page, BufferGetPageSize(buffer));
	pageop = (mbtPageOpaque) PageGetSpecialPointer(page);

	pageop->mbtpo_prev = leftsib;
	pageop->mbtpo_next = rightsib;
	pageop->mbtpo.xact = FrozenTransactionId;
	pageop->mbtpo_flags = mbtP_DELETED;
	pageop->mbtpo_cycleid = 0;

	PageSetLSN(page, lsn);
	PageSetTLI(page, ThisTimeLineID);
	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);

	/* Update metapage if needed */
	if (info == XLOG_mbtREE_DELETE_PAGE_META)
	{
		xl_mbtree_metadata md;

		memcpy(&md, (char *) xlrec + SizeOfmbtreeDeletePage,
			   sizeof(xl_mbtree_metadata));
		_mbt_restore_meta(xlrec->target.node, lsn,
						 md.root, md.level,
						 md.fastroot, md.fastlevel);
	}

	/* Forget any completed deletion */
	forget_matching_deletion(xlrec->target.node, target);

	/* If parent became half-dead, remember it for deletion */
	if (info == XLOG_mbtREE_DELETE_PAGE_HALF)
		log_incomplete_deletion(xlrec->target.node, parent);
}

static void
mbtree_xlog_newroot(XLogRecPtr lsn, XLogRecord *record)
{
	xl_mbtree_newroot *xlrec = (xl_mbtree_newroot *) XLogRecGetData(record);
	Buffer		buffer;
	Page		page;
	mbtPageOpaque pageop;
	BlockNumber downlink = 0;

	buffer = XLogReadBuffer(xlrec->node, xlrec->rootblk, true);
	Assert(BufferIsValid(buffer));
	page = (Page) BufferGetPage(buffer);

	_mbt_pageinit(page, BufferGetPageSize(buffer));
	pageop = (mbtPageOpaque) PageGetSpecialPointer(page);

	pageop->mbtpo_flags = mbtP_ROOT;
	pageop->mbtpo_prev = pageop->mbtpo_next = P_NONE;
	pageop->mbtpo.level = xlrec->level;
	if (xlrec->level == 0)
		pageop->mbtpo_flags |= mbtP_LEAF;
	pageop->mbtpo_cycleid = 0;

	if (record->xl_len > SizeOfmbtreeNewroot)
	{
		IndexTuple	itup;

		_mbt_restore_page(page,
						 (char *) xlrec + SizeOfmbtreeNewroot,
						 record->xl_len - SizeOfmbtreeNewroot);
		/* extract downlink to the right-hand split page */
		itup = (IndexTuple) PageGetItem(page, PageGetItemId(page, P_FIRSTKEY));
		downlink = ItemPointerGetBlockNumber(&(itup->t_tid));
		Assert(ItemPointerGetOffsetNumber(&(itup->t_tid)) == P_HIKEY);
	}

	PageSetLSN(page, lsn);
	PageSetTLI(page, ThisTimeLineID);
	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);

	_mbt_restore_meta(xlrec->node, lsn,
					 xlrec->rootblk, xlrec->level,
					 xlrec->rootblk, xlrec->level);

	/* Check to see if this satisfies any incomplete insertions */
	if (record->xl_len > SizeOfmbtreeNewroot)
		forget_matching_split(xlrec->node, downlink, true);
}


void
mbtree_redo(XLogRecPtr lsn, XLogRecord *record)
{
	uint8		info = record->xl_info & ~XLR_INFO_MASK;

	RestoreBkpBlocks(lsn, record, false);

	switch (info)
	{
		case XLOG_mbtREE_INSERT_LEAF:
			mbtree_xlog_insert(true, false, lsn, record);
			break;
		case XLOG_mbtREE_INSERT_UPPER:
			mbtree_xlog_insert(false, false, lsn, record);
			break;
		case XLOG_mbtREE_INSERT_META:
			mbtree_xlog_insert(false, true, lsn, record);
			break;
		case XLOG_mbtREE_SPLIT_L:
			mbtree_xlog_split(true, false, lsn, record);
			break;
		case XLOG_mbtREE_SPLIT_R:
			mbtree_xlog_split(false, false, lsn, record);
			break;
		case XLOG_mbtREE_SPLIT_L_ROOT:
			mbtree_xlog_split(true, true, lsn, record);
			break;
		case XLOG_mbtREE_SPLIT_R_ROOT:
			mbtree_xlog_split(false, true, lsn, record);
			break;
		case XLOG_mbtREE_DELETE:
			mbtree_xlog_delete(lsn, record);
			break;
		case XLOG_mbtREE_DELETE_PAGE:
		case XLOG_mbtREE_DELETE_PAGE_META:
		case XLOG_mbtREE_DELETE_PAGE_HALF:
			mbtree_xlog_delete_page(info, lsn, record);
			break;
		case XLOG_mbtREE_NEWROOT:
			mbtree_xlog_newroot(lsn, record);
			break;
		default:
			elog(PANIC, "mbtree_redo: unknown op code %u", info);
	}
}

static void
out_target(StringInfo buf, xl_mbtreetid *target)
{
	appendStringInfo(buf, "rel %u/%u/%u; tid %u/%u",
			 target->node.spcNode, target->node.dbNode, target->node.relNode,
					 ItemPointerGetBlockNumber(&(target->tid)),
					 ItemPointerGetOffsetNumber(&(target->tid)));
}

void
mbtree_desc(StringInfo buf, uint8 xl_info, char *rec)
{
	uint8		info = xl_info & ~XLR_INFO_MASK;

	switch (info)
	{
		case XLOG_mbtREE_INSERT_LEAF:
			{
				xl_mbtree_insert *xlrec = (xl_mbtree_insert *) rec;

				appendStringInfo(buf, "insert: ");
				out_target(buf, &(xlrec->target));
				break;
			}
		case XLOG_mbtREE_INSERT_UPPER:
			{
				xl_mbtree_insert *xlrec = (xl_mbtree_insert *) rec;

				appendStringInfo(buf, "insert_upper: ");
				out_target(buf, &(xlrec->target));
				break;
			}
		case XLOG_mbtREE_INSERT_META:
			{
				xl_mbtree_insert *xlrec = (xl_mbtree_insert *) rec;

				appendStringInfo(buf, "insert_meta: ");
				out_target(buf, &(xlrec->target));
				break;
			}
		case XLOG_mbtREE_SPLIT_L:
			{
				xl_mbtree_split *xlrec = (xl_mbtree_split *) rec;

				appendStringInfo(buf, "split_l: rel %u/%u/%u ",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode);
				appendStringInfo(buf, "left %u, right %u, next %u, level %u, firstright %d",
							   xlrec->leftsib, xlrec->rightsib, xlrec->rnext,
								 xlrec->level, xlrec->firstright);
				break;
			}
		case XLOG_mbtREE_SPLIT_R:
			{
				xl_mbtree_split *xlrec = (xl_mbtree_split *) rec;

				appendStringInfo(buf, "split_r: rel %u/%u/%u ",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode);
				appendStringInfo(buf, "left %u, right %u, next %u, level %u, firstright %d",
							   xlrec->leftsib, xlrec->rightsib, xlrec->rnext,
								 xlrec->level, xlrec->firstright);
				break;
			}
		case XLOG_mbtREE_SPLIT_L_ROOT:
			{
				xl_mbtree_split *xlrec = (xl_mbtree_split *) rec;

				appendStringInfo(buf, "split_l_root: rel %u/%u/%u ",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode);
				appendStringInfo(buf, "left %u, right %u, next %u, level %u, firstright %d",
							   xlrec->leftsib, xlrec->rightsib, xlrec->rnext,
								 xlrec->level, xlrec->firstright);
				break;
			}
		case XLOG_mbtREE_SPLIT_R_ROOT:
			{
				xl_mbtree_split *xlrec = (xl_mbtree_split *) rec;

				appendStringInfo(buf, "split_r_root: rel %u/%u/%u ",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode);
				appendStringInfo(buf, "left %u, right %u, next %u, level %u, firstright %d",
							   xlrec->leftsib, xlrec->rightsib, xlrec->rnext,
								 xlrec->level, xlrec->firstright);
				break;
			}
		case XLOG_mbtREE_DELETE:
			{
				xl_mbtree_delete *xlrec = (xl_mbtree_delete *) rec;

				appendStringInfo(buf, "delete: rel %u/%u/%u; blk %u",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode, xlrec->block);
				break;
			}
		case XLOG_mbtREE_DELETE_PAGE:
		case XLOG_mbtREE_DELETE_PAGE_META:
		case XLOG_mbtREE_DELETE_PAGE_HALF:
			{
				xl_mbtree_delete_page *xlrec = (xl_mbtree_delete_page *) rec;

				appendStringInfo(buf, "delete_page: ");
				out_target(buf, &(xlrec->target));
				appendStringInfo(buf, "; dead %u; left %u; right %u",
							xlrec->deadblk, xlrec->leftblk, xlrec->rightblk);
				break;
			}
		case XLOG_mbtREE_NEWROOT:
			{
				xl_mbtree_newroot *xlrec = (xl_mbtree_newroot *) rec;

				appendStringInfo(buf, "newroot: rel %u/%u/%u; root %u lev %u",
								 xlrec->node.spcNode, xlrec->node.dbNode,
								 xlrec->node.relNode,
								 xlrec->rootblk, xlrec->level);
				break;
			}
		default:
			appendStringInfo(buf, "UNKNOWN");
			break;
	}
}

void
mbtree_xlog_startup(void)
{
	incomplete_actions = NIL;
}

void
mbtree_xlog_cleanup(void)
{
	ListCell   *l;

	foreach(l, incomplete_actions)
	{
		mbt_incomplete_action *action = (mbt_incomplete_action *) lfirst(l);

		if (action->is_split)
		{
			/* finish an incomplete split */
			Buffer		lbuf,
						rbuf;
			Page		lpage,
						rpage;
			mbtPageOpaque lpageop,
						rpageop;
			bool		is_only;
			Relation	reln;

			lbuf = XLogReadBuffer(action->node, action->leftblk, false);
			/* failure is impossible because we wrote this page earlier */
			if (!BufferIsValid(lbuf))
				elog(PANIC, "mbtree_xlog_cleanup: left block unfound");
			lpage = (Page) BufferGetPage(lbuf);
			lpageop = (mbtPageOpaque) PageGetSpecialPointer(lpage);
			rbuf = XLogReadBuffer(action->node, action->rightblk, false);
			/* failure is impossible because we wrote this page earlier */
			if (!BufferIsValid(rbuf))
				elog(PANIC, "mbtree_xlog_cleanup: right block unfound");
			rpage = (Page) BufferGetPage(rbuf);
			rpageop = (mbtPageOpaque) PageGetSpecialPointer(rpage);

			/* if the pages are all of their level, it's a only-page split */
			is_only = P_LEFTMOST(lpageop) && P_RIGHTMOST(rpageop);

			reln = CreateFakeRelcacheEntry(action->node);
			_mbt_insert_parent(reln, lbuf, rbuf, NULL,
							  action->is_root, is_only);
			FreeFakeRelcacheEntry(reln);
		}
		else
		{
			/* finish an incomplete deletion (of a half-dead page) */
			Buffer		buf;

			buf = XLogReadBuffer(action->node, action->delblk, false);
			if (BufferIsValid(buf))
			{
				Relation	reln;

				reln = CreateFakeRelcacheEntry(action->node);
				if (_mbt_pagedel(reln, buf, NULL, true) == 0)
					elog(PANIC, "mbtree_xlog_cleanup: _mbt_pagdel failed");
				FreeFakeRelcacheEntry(reln);
			}
		}
	}
	incomplete_actions = NIL;
}

bool
mbtree_safe_restartpoint(void)
{
	if (incomplete_actions)
		return false;
	return true;
}
