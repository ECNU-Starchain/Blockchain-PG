/*-------------------------------------------------------------------------
 *
 * nmbtsort.c
 *		Build a mbtree from sorted input by loading leaf pages sequentially.
 *
 * NOTES
 *
 * We use tuplesort.c to sort the given index tuples into order.
 * Then we scan the index tuples in order and build the mbtree pages
 * for each level.	We load source tuples into leaf-level pages.
 * Whenever we fill a page at one level, we add a link to it to its
 * parent level (starting a new parent level if necessary).  When
 * done, we write out each final page on each level, adding it to
 * its parent level.  When we have only one page on a level, it must be
 * the root -- it can be attached to the mbtree metapage and we are done.
 *
 * This code is moderately slow (~10% slower) compared to the regular
 * mbtree (insertion) build code on sorted or well-clustered data.  On
 * random data, however, the insertion build code is unusable -- the
 * difference on a 60MB heap is a factor of 15 because the random
 * probes into the mbtree thrash the buffer pool.  (NOTE: the above
 * "10%" estimate is probably obsolete, since it refers to an old and
 * not very good external sort implementation that used to exist in
 * this module.  tuplesort.c is almost certainly faster.)
 *
 * It is not wise to pack the pages entirely full, since then *any*
 * insertion would cause a split (and not only of the leaf page; the need
 * for a split would cascade right up the tree).  The steady-state load
 * factor for mbtrees is usually estimated at 70%.  We choose to pack leaf
 * pages to the user-controllable fill factor (default 90%) while upper pages
 * are always packed to 70%.  This gives us reasonable density (there aren't
 * many upper pages if the keys are reasonable-size) without risking a lot of
 * cascading splits during early insertions.
 *
 * Formerly the index pages being built were kept in shared buffers, but
 * that is of no value (since other backends have no interest in them yet)
 * and it created locking problems for CHECKPOINT, because the upper-level
 * pages were held exclusive-locked for long periods.  Now we just build
 * the pages in local memory and smgrwrite or smgrextend them as we finish
 * them.  They will need to be re-read into shared buffers on first use after
 * the build finishes.
 *
 * Since the index will never be used unless it is completely built,
 * from a crash-recovery point of view there is no need to WAL-log the
 * steps of the build.	After completing the index build, we can just sync
 * the whole file to disk using smgrimmedsync() before exiting this module.
 * This can be seen to be sufficient for crash recovery by considering that
 * it's effectively equivalent to what would happen if a CHECKPOINT occurred
 * just after the index build.	However, it is clearly not sufficient if the
 * DBA is using the WAL log for PITR or replication purposes, since another
 * machine would not be able to reconstruct the index from WAL.  Therefore,
 * we log the completed index pages to WAL if and only if WAL archiving is
 * active.
 *
 * This code isn't concerned about the FSM at all. The caller is responsible
 * for initializing that.
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/access/nmbtree/nmbtsort.c,v 1.119 2009/01/01 17:23:36 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/heapam.h"
#include "access/mbtree.h"
#include "miscadmin.h"
#include "storage/smgr.h"
#include "utils/rel.h"
#include "utils/tuplesort.h"


/*
 * Status record for spooling/sorting phase.  (Note we may have two of
 * these due to the special requirements for uniqueness-checking with
 * dead tuples.)
 */
struct mbtSpool
{
	Tuplesortstate *sortstate;	/* state data for tuplesort.c */
	Relation	index;
	bool		isunique;
};

/*
 * Status record for a mbtree page being built.	We have one of these
 * for each active tree level.
 *
 * The reason we need to store a copy of the minimum key is that we'll
 * need to propagate it to the parent node when this page is linked
 * into its parent.  However, if the page is not a leaf page, the first
 * entry on the page doesn't need to contain a key, so we will not have
 * stored the key itself on the page.  (You might think we could skip
 * copying the minimum key on leaf pages, but actually we must have a
 * writable copy anyway because we'll poke the page's address into it
 * before passing it up to the parent...)
 */
typedef struct mbtPageState
{
	Page		mbtps_page;		/* workspace for page building */
	BlockNumber mbtps_blkno;		/* block # to write this page at */
	IndexTuple	mbtps_minkey;	/* copy of minimum key (first item) on page */
	OffsetNumber mbtps_lastoff;	/* last item offset loaded */
	uint32		mbtps_level;		/* tree level (0 = leaf) */
	Size		mbtps_full;		/* "full" if less than this much free space */
	struct mbtPageState *mbtps_next;		/* link to parent level, if any */
} mbtPageState;

/*
 * Overall status record for index writing phase.
 */
typedef struct mbtWriteState
{
	Relation	index;
	bool		mbtws_use_wal;	/* dump pages to WAL? */
	BlockNumber mbtws_pages_alloced;		/* # pages allocated */
	BlockNumber mbtws_pages_written;		/* # pages written out */
	Page		mbtws_zeropage;	/* workspace for filling zeroes */
} mbtWriteState;


static Page _mbt_blnewpage(uint32 level);
static mbtPageState *_mbt_pagestate(mbtWriteState *wstate, uint32 level);
static void _mbt_slideleft(Page page);
static void _mbt_sortaddtup(Page page, Size itemsize,
			   IndexTuple itup, OffsetNumber itup_off);
static void _mbt_buildadd(mbtWriteState *wstate, mbtPageState *state,
			 IndexTuple itup);
static void _mbt_uppershutdown(mbtWriteState *wstate, mbtPageState *state);
static void _mbt_load(mbtWriteState *wstate,
		 mbtSpool *mbtspool, mbtSpool *mbtspool2);


/*
 * Interface routines
 */


/*
 * create and initialize a spool structure
 */
mbtSpool *
_mbt_spoolinit(Relation index, bool isunique, bool isdead)
{
	mbtSpool    *mbtspool = (mbtSpool *) palloc0(sizeof(mbtSpool));
	int			mbtKbytes;

	mbtspool->index = index;
	mbtspool->isunique = isunique;

	/*
	 * We size the sort area as maintenance_work_mem rather than work_mem to
	 * speed index creation.  This should be OK since a single backend can't
	 * run multiple index creations in parallel.  Note that creation of a
	 * unique index actually requires two mbtSpool objects.	We expect that the
	 * second one (for dead tuples) won't get very full, so we give it only
	 * work_mem.
	 */
	mbtKbytes = isdead ? work_mem : maintenance_work_mem;
	mbtspool->sortstate = tuplesort_begin_index_mbt(index, isunique,
													 mbtKbytes, false);

	return mbtspool;
}

/*
 * clean up a spool structure and its substructures.
 */
void
_mbt_spooldestroy(mbtSpool *mbtspool)
{
	tuplesort_end(mbtspool->sortstate);
	pfree(mbtspool);
}

/*
 * spool an index entry into the sort file.
 */
void
_mbt_spool(IndexTuple itup, mbtSpool *mbtspool)
{
	tuplesort_putindextuple(mbtspool->sortstate, itup);
}

/*
 * given a spool loaded by successive calls to _mbt_spool,
 * create an entire mbtree.
 */
void
_mbt_leafbuild(mbtSpool *mbtspool, mbtSpool *mbtspool2)
{
	mbtWriteState wstate;

#ifdef mbtREE_BUILD_STATS
	if (log_mbtree_build_stats)
	{
		ShowUsage("mbtREE BUILD (Spool) STATISTICS");
		ResetUsage();
	}
#endif   /* mbtREE_BUILD_STATS */

	tuplesort_performsort(mbtspool->sortstate);
	if (mbtspool2)
		tuplesort_performsort(mbtspool2->sortstate);

	wstate.index = mbtspool->index;

	/*
	 * We need to log index creation in WAL iff WAL archiving is enabled AND
	 * it's not a temp index.
	 */
	wstate.mbtws_use_wal = XLogArchivingActive() && !wstate.index->rd_istemp;

	/* reserve the metapage */
	wstate.mbtws_pages_alloced = mbtREE_METAPAGE + 1;
	wstate.mbtws_pages_written = 0;
	wstate.mbtws_zeropage = NULL;	/* until needed */

	_mbt_load(&wstate, mbtspool, mbtspool2);
}


/*
 * Internal routines.
 */


/*
 * allocate workspace for a new, clean mbtree page, not linked to any siblings.
 */
static Page
_mbt_blnewpage(uint32 level)
{
	Page		page;
	mbtPageOpaque opaque;

	page = (Page) palloc(BLCKSZ);

	/* Zero the page and set up standard page header info */
	_mbt_pageinit(page, BLCKSZ);

	/* Initialize mbt opaque state */
	opaque = (mbtPageOpaque) PageGetSpecialPointer(page);
	opaque->mbtpo_prev = opaque->mbtpo_next = P_NONE;
	opaque->mbtpo.level = level;
	opaque->mbtpo_flags = (level > 0) ? 0 : mbtP_LEAF;
	opaque->mbtpo_cycleid = 0;

	/* Make the P_HIKEY line pointer appear allocated */
	((PageHeader) page)->pd_lower += sizeof(ItemIdData);

	return page;
}

/*
 * emit a completed mbtree page, and release the working storage.
 */
static void
_mbt_blwritepage(mbtWriteState *wstate, Page page, BlockNumber blkno)
{
	/* Ensure rd_smgr is open (could have been closed by relcache flush!) */
	RelationOpenSmgr(wstate->index);

	/* XLOG stuff */
	if (wstate->mbtws_use_wal)
	{
		/* We use the heap NEWPAGE record type for this */
		log_newpage(&wstate->index->rd_node, MAIN_FORKNUM, blkno, page);
	}
	else
	{
		/* Leave the page LSN zero if not WAL-logged, but set TLI anyway */
		PageSetTLI(page, ThisTimeLineID);
	}

	/*
	 * If we have to write pages nonsequentially, fill in the space with
	 * zeroes until we come back and overwrite.  This is not logically
	 * necessary on standard Unix filesystems (unwritten space will read as
	 * zeroes anyway), but it should help to avoid fragmentation. The dummy
	 * pages aren't WAL-logged though.
	 */
	while (blkno > wstate->mbtws_pages_written)
	{
		if (!wstate->mbtws_zeropage)
			wstate->mbtws_zeropage = (Page) palloc0(BLCKSZ);
		smgrextend(wstate->index->rd_smgr, MAIN_FORKNUM,
				   wstate->mbtws_pages_written++,
				   (char *) wstate->mbtws_zeropage,
				   true);
	}

	/*
	 * Now write the page.	We say isTemp = true even if it's not a temp
	 * index, because there's no need for smgr to schedule an fsync for this
	 * write; we'll do it ourselves before ending the build.
	 */
	if (blkno == wstate->mbtws_pages_written)
	{
		/* extending the file... */
		smgrextend(wstate->index->rd_smgr, MAIN_FORKNUM, blkno,
				   (char *) page, true);
		wstate->mbtws_pages_written++;
	}
	else
	{
		/* overwriting a block we zero-filled before */
		smgrwrite(wstate->index->rd_smgr, MAIN_FORKNUM, blkno,
				  (char *) page, true);
	}

	pfree(page);
}

/*
 * allocate and initialize a new mbtPageState.  the returned structure
 * is suitable for immediate use by _mbt_buildadd.
 */
static mbtPageState *
_mbt_pagestate(mbtWriteState *wstate, uint32 level)
{
	mbtPageState *state = (mbtPageState *) palloc0(sizeof(mbtPageState));

	/* create initial page for level */
	state->mbtps_page = _mbt_blnewpage(level);

	/* and assign it a page position */
	state->mbtps_blkno = wstate->mbtws_pages_alloced++;

	state->mbtps_minkey = NULL;
	/* initialize lastoff so first item goes into P_FIRSTKEY */
	state->mbtps_lastoff = P_HIKEY;
	state->mbtps_level = level;
	/* set "full" threshold based on level.  See notes at head of file. */
	if (level > 0)
		state->mbtps_full = (BLCKSZ * (100 - mbt_NONLEAF_FILLFACTOR) / 100);
	else
		state->mbtps_full = RelationGetTargetPageFreeSpace(wstate->index,
												   mbt_DEFAULT_FILLFACTOR);
	/* no parent level, yet */
	state->mbtps_next = NULL;

	return state;
}

/*
 * slide an array of ItemIds back one slot (from P_FIRSTKEY to
 * P_HIKEY, overwriting P_HIKEY).  we need to do this when we discover
 * that we have built an ItemId array in what has turned out to be a
 * P_RIGHTMOST page.
 */
static void
_mbt_slideleft(Page page)
{
	OffsetNumber off;
	OffsetNumber maxoff;
	ItemId		previi;
	ItemId		thisii;

	if (!PageIsEmpty(page))
	{
		maxoff = PageGetMaxOffsetNumber(page);
		previi = PageGetItemId(page, P_HIKEY);
		for (off = P_FIRSTKEY; off <= maxoff; off = OffsetNumberNext(off))
		{
			thisii = PageGetItemId(page, off);
			*previi = *thisii;
			previi = thisii;
		}
		((PageHeader) page)->pd_lower -= sizeof(ItemIdData);
	}
}

/*
 * Add an item to a page being built.
 *
 * The main difference between this routine and a bare PageAddItem call
 * is that this code knows that the leftmost data item on a non-leaf
 * mbtree page doesn't need to have a key.  Therefore, it strips such
 * items down to just the item header.
 *
 * This is almost like nmbtinsert.c's _mbt_pgaddtup(), but we can't use
 * that because it assumes that P_RIGHTMOST() will return the correct
 * answer for the page.  Here, we don't know yet if the page will be
 * rightmost.  Offset P_FIRSTKEY is always the first data key.
 */
static void
_mbt_sortaddtup(Page page,
			   Size itemsize,
			   IndexTuple itup,
			   OffsetNumber itup_off)
{
	mbtPageOpaque opaque = (mbtPageOpaque) PageGetSpecialPointer(page);
	IndexTupleData trunctuple;

	if (!P_ISLEAF(opaque) && itup_off == P_FIRSTKEY)
	{
		trunctuple = *itup;
		trunctuple.t_info = sizeof(IndexTupleData);
		itup = &trunctuple;
		itemsize = sizeof(IndexTupleData);
	}

	if (PageAddItem(page, (Item) itup, itemsize, itup_off,
					false, false) == InvalidOffsetNumber)
		elog(ERROR, "failed to add item to the index page");
}

/*----------
 * Add an item to a disk page from the sort output.
 *
 * We must be careful to observe the page layout conventions of nmbtsearch.c:
 * - rightmost pages start data items at P_HIKEY instead of at P_FIRSTKEY.
 * - on non-leaf pages, the key portion of the first item need not be
 *	 stored, we should store only the link.
 *
 * A leaf page being built looks like:
 *
 * +----------------+---------------------------------+
 * | PageHeaderData | linp0 linp1 linp2 ...			  |
 * +-----------+----+---------------------------------+
 * | ... linpN |									  |
 * +-----------+--------------------------------------+
 * |	 ^ last										  |
 * |												  |
 * +-------------+------------------------------------+
 * |			 | itemN ...						  |
 * +-------------+------------------+-----------------+
 * |		  ... item3 item2 item1 | "special space" |
 * +--------------------------------+-----------------+
 *
 * Contrast this with the diagram in bufpage.h; note the mismatch
 * between linps and items.  This is because we reserve linp0 as a
 * placeholder for the pointer to the "high key" item; when we have
 * filled up the page, we will set linp0 to point to itemN and clear
 * linpN.  On the other hand, if we find this is the last (rightmost)
 * page, we leave the items alone and slide the linp array over.
 *
 * 'last' pointer indicates the last offset added to the page.
 *----------
 */
static void
_mbt_buildadd(mbtWriteState *wstate, mbtPageState *state, IndexTuple itup)
{
	Page		npage;
	BlockNumber nblkno;
	OffsetNumber last_off;
	Size		pgspc;
	Size		itupsz;

	/*
	 * This is a handy place to check for cancel interrupts during the mbtree
	 * load phase of index creation.
	 */
	CHECK_FOR_INTERRUPTS();

	npage = state->mbtps_page;
	nblkno = state->mbtps_blkno;
	last_off = state->mbtps_lastoff;

	pgspc = PageGetFreeSpace(npage);
	itupsz = IndexTupleDSize(*itup);
	itupsz = MAXALIGN(itupsz);

	/*
	 * Check whether the item can fit on a mbtree page at all. (Eventually, we
	 * ought to try to apply TOAST methods if not.) We actually need to be
	 * able to fit three items on every page, so restrict any one item to 1/3
	 * the per-page available space. Note that at this point, itupsz doesn't
	 * include the ItemId.
	 *
	 * NOTE: similar code appears in _mbt_insertonpg() to defend against
	 * oversize items being inserted into an already-existing index. But
	 * during creation of an index, we don't go through there.
	 */
	if (itupsz > mbtMaxItemSize(npage))
		ereport(ERROR,
				(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
				 errmsg("index row size %lu exceeds mbtree maximum, %lu",
						(unsigned long) itupsz,
						(unsigned long) mbtMaxItemSize(npage)),
		errhint("Values larger than 1/3 of a buffer page cannot be indexed.\n"
				"Consider a function index of an MD5 hash of the value, "
				"or use full text indexing.")));

	/*
	 * Check to see if page is "full".	It's definitely full if the item won't
	 * fit.  Otherwise, compare to the target freespace derived from the
	 * fillfactor.	However, we must put at least two items on each page, so
	 * disregard fillfactor if we don't have that many.
	 */
	if (pgspc < itupsz || (pgspc < state->mbtps_full && last_off > P_FIRSTKEY))
	{
		/*
		 * Finish off the page and write it out.
		 */
		Page		opage = npage;
		BlockNumber oblkno = nblkno;
		ItemId		ii;
		ItemId		hii;
		IndexTuple	oitup;

		/* Create new page of same level */
		npage = _mbt_blnewpage(state->mbtps_level);

		/* and assign it a page position */
		nblkno = wstate->mbtws_pages_alloced++;

		/*
		 * We copy the last item on the page into the new page, and then
		 * rearrange the old page so that the 'last item' becomes its high key
		 * rather than a true data item.  There had better be at least two
		 * items on the page already, else the page would be embty of useful
		 * data.
		 */
		Assert(last_off > P_FIRSTKEY);
		ii = PageGetItemId(opage, last_off);
		oitup = (IndexTuple) PageGetItem(opage, ii);
		_mbt_sortaddtup(npage, ItemIdGetLength(ii), oitup, P_FIRSTKEY);

		/*
		 * Move 'last' into the high key position on opage
		 */
		hii = PageGetItemId(opage, P_HIKEY);
		*hii = *ii;
		ItemIdSetUnused(ii);	/* redundant */
		((PageHeader) opage)->pd_lower -= sizeof(ItemIdData);

		/*
		 * Link the old page into its parent, using its minimum key. If we
		 * don't have a parent, we have to create one; this adds a new mbtree
		 * level.
		 */
		if (state->mbtps_next == NULL)
			state->mbtps_next = _mbt_pagestate(wstate, state->mbtps_level + 1);

		Assert(state->mbtps_minkey != NULL);
		ItemPointerSet(&(state->mbtps_minkey->t_tid), oblkno, P_HIKEY);
		_mbt_buildadd(wstate, state->mbtps_next, state->mbtps_minkey);
		pfree(state->mbtps_minkey);

		/*
		 * Save a copy of the minimum key for the new page.  We have to copy
		 * it off the old page, not the new one, in case we are not at leaf
		 * level.
		 */
		state->mbtps_minkey = CopyIndexTuple(oitup);

		/*
		 * Set the sibling links for both pages.
		 */
		{
			mbtPageOpaque oopaque = (mbtPageOpaque) PageGetSpecialPointer(opage);
			mbtPageOpaque nopaque = (mbtPageOpaque) PageGetSpecialPointer(npage);

			oopaque->mbtpo_next = nblkno;
			nopaque->mbtpo_prev = oblkno;
			nopaque->mbtpo_next = P_NONE;		/* redundant */
		}

		/*
		 * Write out the old page.	We never need to touch it again, so we can
		 * free the opage workspace too.
		 */
		_mbt_blwritepage(wstate, opage, oblkno);

		/*
		 * Reset last_off to point to new page
		 */
		last_off = P_FIRSTKEY;
	}

	/*
	 * If the new item is the first for its page, stash a copy for later. Note
	 * this will only happen for the first item on a level; on later pages,
	 * the first item for a page is copied from the prior page in the code
	 * above.
	 */
	if (last_off == P_HIKEY)
	{
		Assert(state->mbtps_minkey == NULL);
		state->mbtps_minkey = CopyIndexTuple(itup);
	}

	/*
	 * Add the new item into the current page.
	 */
	last_off = OffsetNumberNext(last_off);
	_mbt_sortaddtup(npage, itupsz, itup, last_off);

	state->mbtps_page = npage;
	state->mbtps_blkno = nblkno;
	state->mbtps_lastoff = last_off;
}

/*
 * Finish writing out the completed mbtree.
 */
static void
_mbt_uppershutdown(mbtWriteState *wstate, mbtPageState *state)
{
	mbtPageState *s;
	BlockNumber rootblkno = P_NONE;
	uint32		rootlevel = 0;
	Page		metapage;

	/*
	 * Each iteration of this loop completes one more level of the tree.
	 */
	for (s = state; s != NULL; s = s->mbtps_next)
	{
		BlockNumber blkno;
		mbtPageOpaque opaque;

		blkno = s->mbtps_blkno;
		opaque = (mbtPageOpaque) PageGetSpecialPointer(s->mbtps_page);

		/*
		 * We have to link the last page on this level to somewhere.
		 *
		 * If we're at the top, it's the root, so attach it to the metapage.
		 * Otherwise, add an entry for it to its parent using its minimum key.
		 * This may cause the last page of the parent level to split, but
		 * that's not a problem -- we haven't gotten to it yet.
		 */
		if (s->mbtps_next == NULL)
		{
			opaque->mbtpo_flags |= mbtP_ROOT;
			rootblkno = blkno;
			rootlevel = s->mbtps_level;
		}
		else
		{
			Assert(s->mbtps_minkey != NULL);
			ItemPointerSet(&(s->mbtps_minkey->t_tid), blkno, P_HIKEY);
			_mbt_buildadd(wstate, s->mbtps_next, s->mbtps_minkey);
			pfree(s->mbtps_minkey);
			s->mbtps_minkey = NULL;
		}

		/*
		 * This is the rightmost page, so the ItemId array needs to be slid
		 * back one slot.  Then we can dump out the page.
		 */
		_mbt_slideleft(s->mbtps_page);
		_mbt_blwritepage(wstate, s->mbtps_page, s->mbtps_blkno);
		s->mbtps_page = NULL;	/* writepage freed the workspace */
	}

	/*
	 * As the last step in the process, construct the metapage and make it
	 * point to the new root (unless we had no data at all, in which case it's
	 * set to point to "P_NONE").  This changes the index to the "valid" state
	 * by filling in a valid magic number in the metapage.
	 */
	metapage = (Page) palloc(BLCKSZ);
	_mbt_initmetapage(metapage, rootblkno, rootlevel);
	_mbt_blwritepage(wstate, metapage, mbtREE_METAPAGE);
}

/*
 * Read tuples in correct sort order from tuplesort, and load them into
 * mbtree leaves.
 */
static void
_mbt_load(mbtWriteState *wstate, mbtSpool *mbtspool, mbtSpool *mbtspool2)
{
	mbtPageState *state = NULL;
	bool		merge = (mbtspool2 != NULL);
	IndexTuple	itup,
				itup2 = NULL;
	bool		should_free,
				should_free2,
				load1;
	TupleDesc	tupdes = RelationGetDescr(wstate->index);
	int			i,
				keysz = RelationGetNumberOfAttributes(wstate->index);
	ScanKey		indexScanKey = NULL;

	if (merge)
	{
		/*
		 * Another mbtSpool for dead tuples exists. Now we have to merge
		 * mbtspool and mbtspool2.
		 */

		/* the preparation of merge */
		itup = tuplesort_getindextuple(mbtspool->sortstate,
									   true, &should_free);
		itup2 = tuplesort_getindextuple(mbtspool2->sortstate,
										true, &should_free2);
		indexScanKey = _mbt_mkscankey_nodata(wstate->index);

		for (;;)
		{
			load1 = true;		/* load mbtSpool next ? */
			if (itup2 == NULL)
			{
				if (itup == NULL)
					break;
			}
			else if (itup != NULL)
			{
				for (i = 1; i <= keysz; i++)
				{
					ScanKey		entry;
					Datum		attrDatum1,
								attrDatum2;
					bool		isNull1,
								isNull2;
					int32		compare;

					entry = indexScanKey + i - 1;
					attrDatum1 = index_getattr(itup, i, tupdes, &isNull1);
					attrDatum2 = index_getattr(itup2, i, tupdes, &isNull2);
					if (isNull1)
					{
						if (isNull2)
							compare = 0;		/* NULL "=" NULL */
						else if (entry->sk_flags & SK_mbt_NULLS_FIRST)
							compare = -1;		/* NULL "<" NOT_NULL */
						else
							compare = 1;		/* NULL ">" NOT_NULL */
					}
					else if (isNull2)
					{
						if (entry->sk_flags & SK_mbt_NULLS_FIRST)
							compare = 1;		/* NOT_NULL ">" NULL */
						else
							compare = -1;		/* NOT_NULL "<" NULL */
					}
					else
					{
						compare = DatumGetInt32(FunctionCall2(&entry->sk_func,
															  attrDatum1,
															  attrDatum2));

						if (entry->sk_flags & SK_mbt_DESC)
							compare = -compare;
					}
					if (compare > 0)
					{
						load1 = false;
						break;
					}
					else if (compare < 0)
						break;
				}
			}
			else
				load1 = false;

			/* When we see first tuple, create first index page */
			if (state == NULL)
				state = _mbt_pagestate(wstate, 0);

			if (load1)
			{
				_mbt_buildadd(wstate, state, itup);
				if (should_free)
					pfree(itup);
				itup = tuplesort_getindextuple(mbtspool->sortstate,
											   true, &should_free);
			}
			else
			{
				_mbt_buildadd(wstate, state, itup2);
				if (should_free2)
					pfree(itup2);
				itup2 = tuplesort_getindextuple(mbtspool2->sortstate,
												true, &should_free2);
			}
		}
		_mbt_freeskey(indexScanKey);
	}
	else
	{
		/* merge is unnecessary */
		while ((itup = tuplesort_getindextuple(mbtspool->sortstate,
											   true, &should_free)) != NULL)
		{
			/* When we see first tuple, create first index page */
			if (state == NULL)
				state = _mbt_pagestate(wstate, 0);

			_mbt_buildadd(wstate, state, itup);
			if (should_free)
				pfree(itup);
		}
	}

	/* Close down final pages and write the metapage */
	_mbt_uppershutdown(wstate, state);

	/*
	 * If the index isn't temp, we must fsync it down to disk before it's safe
	 * to commit the transaction.  (For a temp index we don't care since the
	 * index will be uninteresting after a crash anyway.)
	 *
	 * It's obvious that we must do this when not WAL-logging the build. It's
	 * less obvious that we have to do it even if we did WAL-log the index
	 * pages.  The reason is that since we're building outside shared buffers,
	 * a CHECKPOINT occurring during the build has no way to flush the
	 * previously written data to disk (indeed it won't know the index even
	 * exists).  A crash later on would replay WAL from the checkpoint,
	 * therefore it wouldn't replay our earlier WAL entries. If we do not
	 * fsync those pages here, they might still not be on disk when the crash
	 * occurs.
	 */
	if (!wstate->index->rd_istemp)
	{
		RelationOpenSmgr(wstate->index);
		smgrimmedsync(wstate->index->rd_smgr, MAIN_FORKNUM);
	}
}
