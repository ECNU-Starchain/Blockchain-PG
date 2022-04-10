/*-------------------------------------------------------------------------
 *
 * nmbtree.h
 *	  header file for postgres mbtree access method implementation.
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/nmbtree.h,v 1.124 2009/06/11 14:49:08 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef mbtREE_H
#define mbtREE_H

#include "access/genam.h"
#include "access/itup.h"
#include "access/sdir.h"
#include "access/xlog.h"
#include "access/xlogutils.h"


/* There's room for a 16-bit vacuum cycle ID in mbtPageOpaqueData */
typedef uint16 mbtCycleId;

/*
 *	mbtPageOpaqueData -- At the end of every page, we store a pointer
 *	to both siblings in the tree.  This is used to do forward/backward
 *	index scans.  The next-page link is also critical for recovery when
 *	a search has navigated to the wrong page due to concurrent page splits
 *	or deletions; see src/backend/access/nmbtree/README for more info.
 *
 *	In addition, we store the page's mbtree level (counting upwards from
 *	zero at a leaf page) as well as some flag bits indicating the page type
 *	and status.  If the page is deleted, we replace the level with the
 *	next-transaction-ID value indicating when it is safe to reclaim the page.
 *
 *	We also store a "vacuum cycle ID".	When a page is split while VACUUM is
 *	processing the index, a nonzero value associated with the VACUUM run is
 *	stored into both halves of the split page.	(If VACUUM is not running,
 *	both pages receive zero cycleids.)	This allows VACUUM to detect whether
 *	a page was split since it started, with a small probability of false match
 *	if the page was last split some exact multiple of MAX_mbt_CYCLE_ID VACUUMs
 *	ago.  Also, during a split, the mbtP_SPLIT_END flag is cleared in the left
 *	(original) page, and set in the right page, but only if the next page
 *	to its right has a different cycleid.
 *
 *	NOTE: the mbtP_LEAF flag bit is redundant since level==0 could be tested
 *	instead.
 */

typedef struct mbtPageOpaqueData
{
	BlockNumber mbtpo_prev;		/* left sibling, or P_NONE if leftmost */
	BlockNumber mbtpo_next;		/* right sibling, or P_NONE if rightmost */
	union
	{
		uint32		level;		/* tree level --- zero for leaf pages */
		TransactionId xact;		/* next transaction ID, if deleted */
	}			mbtpo;
	uint16		mbtpo_flags;		/* flag bits, see below */
	mbtCycleId	mbtpo_cycleid;	/* vacuum cycle ID of latest split */
} mbtPageOpaqueData;

typedef mbtPageOpaqueData *mbtPageOpaque;

/* Bits defined in mbtpo_flags */
#define mbtP_LEAF		(1 << 0)	/* leaf page, i.e. not internal page */
#define mbtP_ROOT		(1 << 1)	/* root page (has no parent) */
#define mbtP_DELETED		(1 << 2)	/* page has been deleted from tree */
#define mbtP_META		(1 << 3)	/* meta-page */
#define mbtP_HALF_DEAD	(1 << 4)	/* embty, but still in tree */
#define mbtP_SPLIT_END	(1 << 5)	/* rightmost page of split group */
#define mbtP_HAS_GARBAGE (1 << 6)	/* page has LP_DEAD tuples */

/*
 * The max allowed value of a cycle ID is a bit less than 64K.	This is
 * for convenience of pg_filedump and similar utilities: we want to use
 * the last 2 bytes of special space as an index type indicator, and
 * restricting cycle ID lets mbtree use that space for vacuum cycle IDs
 * while still allowing index type to be identified.
 */
#define MAX_mbt_CYCLE_ID		0xFF7F


/*
 * The Meta page is always the first page in the mbtree index.
 * Its primary purpose is to point to the location of the mbtree root page.
 * We also point to the "fast" root, which is the current effective root;
 * see README for discussion.
 */

typedef struct mbtMetaPageData
{
	uint32		mbtm_magic;		/* should contain mbtREE_MAGIC */
	uint32		mbtm_version;	/* should contain mbtREE_VERSION */
	BlockNumber mbtm_root;		/* current root location */
	uint32		mbtm_level;		/* tree level of the root page */
	BlockNumber mbtm_fastroot;	/* current "fast" root location */
	uint32		mbtm_fastlevel;	/* tree level of the "fast" root page */
} mbtMetaPageData;

#define mbtPageGetMeta(p) \
	((mbtMetaPageData *) PageGetContents(p))

#define mbtREE_METAPAGE	0		/* first page is meta */
#define mbtREE_MAGIC		0x053162	/* magic number of mbtree pages */
#define mbtREE_VERSION	2		/* current version number */

/*
 * Maximum size of a mbtree index entry, including its tuple header.
 *
 * We actually need to be able to fit three items on every page,
 * so restrict any one item to 1/3 the per-page available space.
 */
#define mbtMaxItemSize(page) \
	MAXALIGN_DOWN((PageGetPageSize(page) - \
				   MAXALIGN(SizeOfPageHeaderData + 3*sizeof(ItemIdData)) - \
				   MAXALIGN(sizeof(mbtPageOpaqueData))) / 3)

/*
 * The leaf-page fillfactor defaults to 90% but is user-adjustable.
 * For pages above the leaf level, we use a fixed 70% fillfactor.
 * The fillfactor is applied during index build and when splitting
 * a rightmost page; when splitting non-rightmost pages we try to
 * divide the data equally.
 */
#define mbt_MIN_FILLFACTOR		10
#define mbt_DEFAULT_FILLFACTOR	90
#define mbt_NONLEAF_FILLFACTOR	70

/*
 *	Test whether two mbtree entries are "the same".
 *
 *	Old comments:
 *	In addition, we must guarantee that all tuples in the index are unique,
 *	in order to satisfy some assumbtions in Lehman and Yao.  The way that we
 *	do this is by generating a new OID for every insertion that we do in the
 *	tree.  This adds eight bytes to the size of mbtree index tuples.  Note
 *	that we do not use the OID as part of a composite key; the OID only
 *	serves as a unique identifier for a given index tuple (logical position
 *	within a page).
 *
 *	New comments:
 *	actually, we must guarantee that all tuples in A LEVEL
 *	are unique, not in ALL INDEX. So, we can use the t_tid
 *	as unique identifier for a given index tuple (logical position
 *	within a level). - vadim 04/09/97
 */
#define mbtTidSame(i1, i2)	\
	( (i1).ip_blkid.bi_hi == (i2).ip_blkid.bi_hi && \
	  (i1).ip_blkid.bi_lo == (i2).ip_blkid.bi_lo && \
	  (i1).ip_posid == (i2).ip_posid )
#define mbtEntrySame(i1, i2) \
	mbtTidSame((i1)->t_tid, (i2)->t_tid)


/*
 *	In general, the mbtree code tries to localize its knowledge about
 *	page layout to a couple of routines.  However, we need a special
 *	value to indicate "no page number" in those places where we expect
 *	page numbers.  We can use zero for this because we never need to
 *	make a pointer to the metadata page.
 */

#define P_NONE			0

/*
 * Macros to test whether a page is leftmost or rightmost on its tree level,
 * as well as other state info kept in the opaque data.
 */
#define P_LEFTMOST(opaque)		((opaque)->mbtpo_prev == P_NONE)
#define P_RIGHTMOST(opaque)		((opaque)->mbtpo_next == P_NONE)
#define P_ISLEAF(opaque)		((opaque)->mbtpo_flags & mbtP_LEAF)
#define P_ISROOT(opaque)		((opaque)->mbtpo_flags & mbtP_ROOT)
#define P_ISDELETED(opaque)		((opaque)->mbtpo_flags & mbtP_DELETED)
#define P_ISHALFDEAD(opaque)	((opaque)->mbtpo_flags & mbtP_HALF_DEAD)
#define P_IGNORE(opaque)		((opaque)->mbtpo_flags & (mbtP_DELETED|mbtP_HALF_DEAD))
#define P_HAS_GARBAGE(opaque)	((opaque)->mbtpo_flags & mbtP_HAS_GARBAGE)

/*
 *	Lehman and Yao's algorithm requires a ``high key'' on every non-rightmost
 *	page.  The high key is not a data key, but gives info about what range of
 *	keys is supposed to be on this page.  The high key on a page is required
 *	to be greater than or equal to any data key that appears on the page.
 *	If we find ourselves trying to insert a key > high key, we know we need
 *	to move right (this should only happen if the page was split since we
 *	examined the parent page).
 *
 *	Our insertion algorithm guarantees that we can use the initial least key
 *	on our right sibling as the high key.  Once a page is created, its high
 *	key changes only if the page is split.
 *
 *	On a non-rightmost page, the high key lives in item 1 and data items
 *	start in item 2.  Rightmost pages have no high key, so we store data
 *	items beginning in item 1.
 */

#define P_HIKEY				((OffsetNumber) 1)
#define P_FIRSTKEY			((OffsetNumber) 2)
#define P_FIRSTDATAKEY(opaque)	(P_RIGHTMOST(opaque) ? P_HIKEY : P_FIRSTKEY)

/*
 * XLOG records for mbtree operations
 *
 * XLOG allows to store some information in high 4 bits of log
 * record xl_info field
 */
#define XLOG_mbtREE_INSERT_LEAF	0x00	/* add index tuple without split */
#define XLOG_mbtREE_INSERT_UPPER 0x10	/* same, on a non-leaf page */
#define XLOG_mbtREE_INSERT_META	0x20	/* same, plus update metapage */
#define XLOG_mbtREE_SPLIT_L		0x30	/* add index tuple with split */
#define XLOG_mbtREE_SPLIT_R		0x40	/* as above, new item on right */
#define XLOG_mbtREE_SPLIT_L_ROOT 0x50	/* add tuple with split of root */
#define XLOG_mbtREE_SPLIT_R_ROOT 0x60	/* as above, new item on right */
#define XLOG_mbtREE_DELETE		0x70	/* delete leaf index tuple */
#define XLOG_mbtREE_DELETE_PAGE	0x80	/* delete an entire page */
#define XLOG_mbtREE_DELETE_PAGE_META 0x90		/* same, and update metapage */
#define XLOG_mbtREE_NEWROOT		0xA0	/* new root page */
#define XLOG_mbtREE_DELETE_PAGE_HALF 0xB0		/* page deletion that makes
												 * parent half-dead */

/*
 * All that we need to find changed index tuple
 */
typedef struct xl_mbtreetid
{
	RelFileNode node;
	ItemPointerData tid;		/* changed tuple id */
} xl_mbtreetid;

/*
 * All that we need to regenerate the meta-data page
 */
typedef struct xl_mbtree_metadata
{
	BlockNumber root;
	uint32		level;
	BlockNumber fastroot;
	uint32		fastlevel;
} xl_mbtree_metadata;

/*
 * This is what we need to know about simple (without split) insert.
 *
 * This data record is used for INSERT_LEAF, INSERT_UPPER, INSERT_META.
 * Note that INSERT_META implies it's not a leaf page.
 */
typedef struct xl_mbtree_insert
{
	xl_mbtreetid target;			/* inserted tuple id */
	/* BlockNumber downlink field FOLLOWS IF NOT XLOG_mbtREE_INSERT_LEAF */
	/* xl_mbtree_metadata FOLLOWS IF XLOG_mbtREE_INSERT_META */
	/* INDEX TUPLE FOLLOWS AT END OF STRUCT */
} xl_mbtree_insert;

#define SizeOfmbtreeInsert	(offsetof(xl_mbtreetid, tid) + SizeOfIptrData)

/*
 * On insert with split, we save all the items going into the right sibling
 * so that we can restore it completely from the log record.  This way takes
 * less xlog space than the normal approach, because if we did it standardly,
 * XLogInsert would almost always think the right page is new and store its
 * whole page image.  The left page, however, is handled in the normal
 * incremental-update fashion.
 *
 * Note: the four XLOG_mbtREE_SPLIT xl_info codes all use this data record.
 * The _L and _R variants indicate whether the inserted tuple went into the
 * left or right split page (and thus, whether newitemoff and the new item
 * are stored or not).	The _ROOT variants indicate that we are splitting
 * the root page, and thus that a newroot record rather than an insert or
 * split record should follow.	Note that a split record never carries a
 * metapage update --- we'll do that in the parent-level update.
 */
typedef struct xl_mbtree_split
{
	RelFileNode node;
	BlockNumber leftsib;		/* orig page / new left page */
	BlockNumber rightsib;		/* new right page */
	BlockNumber rnext;			/* next block (orig page's rightlink) */
	uint32		level;			/* tree level of page being split */
	OffsetNumber firstright;	/* first item moved to right page */

	/*
	 * If level > 0, BlockIdData downlink follows.	(We use BlockIdData rather
	 * than BlockNumber for alignment reasons: SizeOfmbtreeSplit is only 16-bit
	 * aligned.)
	 *
	 * If level > 0, an IndexTuple representing the HIKEY of the left page
	 * follows.  We don't need this on leaf pages, because it's the same as
	 * the leftmost key in the new right page.	Also, it's suppressed if
	 * XLogInsert chooses to store the left page's whole page image.
	 *
	 * In the _L variants, next are OffsetNumber newitemoff and the new item.
	 * (In the _R variants, the new item is one of the right page's tuples.)
	 * The new item, but not newitemoff, is suppressed if XLogInsert chooses
	 * to store the left page's whole page image.
	 *
	 * Last are the right page's tuples in the form used by _mbt_restore_page.
	 */
} xl_mbtree_split;

#define SizeOfmbtreeSplit	(offsetof(xl_mbtree_split, firstright) + sizeof(OffsetNumber))

/*
 * This is what we need to know about delete of individual leaf index tuples.
 * The WAL record can represent deletion of any number of index tuples on a
 * single index page.
 */
typedef struct xl_mbtree_delete
{
	RelFileNode node;
	BlockNumber block;
	/* TARGET OFFSET NUMBERS FOLLOW AT THE END */
} xl_mbtree_delete;

#define SizeOfmbtreeDelete	(offsetof(xl_mbtree_delete, block) + sizeof(BlockNumber))

/*
 * This is what we need to know about deletion of a mbtree page.  The target
 * identifies the tuple removed from the parent page (note that we remove
 * this tuple's downlink and the *following* tuple's key).	Note we do not
 * store any content for the deleted page --- it is just rewritten as embty
 * during recovery.
 */
typedef struct xl_mbtree_delete_page
{
	xl_mbtreetid target;			/* deleted tuple id in parent page */
	BlockNumber deadblk;		/* child block being deleted */
	BlockNumber leftblk;		/* child block's left sibling, if any */
	BlockNumber rightblk;		/* child block's right sibling */
	/* xl_mbtree_metadata FOLLOWS IF XLOG_mbtREE_DELETE_PAGE_META */
} xl_mbtree_delete_page;

#define SizeOfmbtreeDeletePage	(offsetof(xl_mbtree_delete_page, rightblk) + sizeof(BlockNumber))

/*
 * New root log record.  There are zero tuples if this is to establish an
 * embty root, or two if it is the result of splitting an old root.
 *
 * Note that although this implies rewriting the metadata page, we don't need
 * an xl_mbtree_metadata record --- the rootblk and level are sufficient.
 */
typedef struct xl_mbtree_newroot
{
	RelFileNode node;
	BlockNumber rootblk;		/* location of new root */
	uint32		level;			/* its tree level */
	/* 0 or 2 INDEX TUPLES FOLLOW AT END OF STRUCT */
} xl_mbtree_newroot;

#define SizeOfmbtreeNewroot	(offsetof(xl_mbtree_newroot, level) + sizeof(uint32))


/*
 *	Operator strategy numbers for B-tree have been moved to access/skey.h,
 *	because many places need to use them in ScanKeyInit() calls.
 *
 *	The strategy numbers are chosen so that we can commute them by
 *	sumbtraction, thus:
 */
#define mbtCommuteStrategyNumber(strat)	(mbtMaxStrategyNumber + 1 - (strat))

/*
 *	When a new operator class is declared, we require that the user
 *	supply us with an amproc procedure for determining whether, for
 *	two keys a and b, a < b, a = b, or a > b.  This routine must
 *	return < 0, 0, > 0, respectively, in these three cases.  Since we
 *	only have one such proc in amproc, it's number 1.
 */

#define mbtORDER_PROC	1

/*
 *	We need to be able to tell the difference between read and write
 *	requests for pages, in order to do locking correctly.
 */

#define mbt_READ			BUFFER_LOCK_SHARE
#define mbt_WRITE		BUFFER_LOCK_EXCLUSIVE

/*
 *	mbtStackData -- As we descend a tree, we push the (location, downlink)
 *	pairs from internal pages onto a private stack.  If we split a
 *	leaf, we use this stack to walk back up the tree and insert data
 *	into parent pages (and possibly to split them, too).  Lehman and
 *	Yao's update algorithm guarantees that under no circumstances can
 *	our private stack give us an irredeemably bad picture up the tree.
 *	Again, see the paper for details.
 */

typedef struct mbtStackData
{
	BlockNumber mbts_blkno;
	OffsetNumber mbts_offset;
	IndexTupleData mbts_mbtentry;
	struct mbtStackData *mbts_parent;
} mbtStackData;

typedef mbtStackData *mbtStack;

/*
 * mbtScanOpaqueData is the mbtree-private state needed for an indexscan.
 * This consists of preprocessed scan keys (see _mbt_preprocess_keys() for
 * details of the preprocessing), information about the current location
 * of the scan, and information about the marked location, if any.	(We use
 * mbtScanPosData to represent the data needed for each of current and marked
 * locations.)	In addition we can remember some known-killed index entries
 * that must be marked before we can move off the current page.
 *
 * Index scans work a page at a time: we pin and read-lock the page, identify
 * all the matching items on the page and save them in mbtScanPosData, then
 * release the read-lock while returning the items to the caller for
 * processing.	This approach minimizes lock/unlock traffic.  Note that we
 * keep the pin on the index page until the caller is done with all the items
 * (this is needed for VACUUM synchronization, see nmbtree/README).	When we
 * are ready to step to the next page, if the caller has told us any of the
 * items were killed, we re-lock the page to mark them killed, then unlock.
 * Finally we drop the pin and step to the next page in the appropriate
 * direction.
 */

typedef struct mbtScanPosItem	/* what we remember about each match */
{
	ItemPointerData heapTid;	/* TID of referenced heap item */
	OffsetNumber indexOffset;	/* index item's location within page */
} mbtScanPosItem;

typedef struct mbtScanPosData
{
	Buffer		buf;			/* if valid, the buffer is pinned */

	BlockNumber nextPage;		/* page's right link when we scanned it */

	/*
	 * moreLeft and moreRight track whether we think there may be matching
	 * index entries to the left and right of the current page, respectively.
	 * We can clear the appropriate one of these flags when _mbt_checkkeys()
	 * returns continuescan = false.
	 */
	bool		moreLeft;
	bool		moreRight;

	/*
	 * The items array is always ordered in index order (ie, increasing
	 * indexoffset).  When scanning backwards it is convenient to fill the
	 * array back-to-front, so we start at the last slot and fill downwards.
	 * Hence we need both a first-valid-entry and a last-valid-entry counter.
	 * itemIndex is a cursor showing which entry was last returned to caller.
	 */
	int			firstItem;		/* first valid index in items[] */
	int			lastItem;		/* last valid index in items[] */
	int			itemIndex;		/* current index in items[] */

	mbtScanPosItem items[MaxIndexTuplesPerPage]; /* MUST BE LAST */
} mbtScanPosData;

typedef mbtScanPosData *mbtScanPos;

#define mbtScanPosIsValid(scanpos) BufferIsValid((scanpos).buf)

typedef struct mbtScanOpaqueData
{
	/* these fields are set by _mbt_preprocess_keys(): */
	bool		qual_ok;		/* false if qual can never be satisfied */
	int			numberOfKeys;	/* number of preprocessed scan keys */
	ScanKey		keyData;		/* array of preprocessed scan keys */

	/* info about killed items if any (killedItems is NULL if never used) */
	int		   *killedItems;	/* currPos.items indexes of killed items */
	int			numKilled;		/* number of currently stored items */

	/*
	 * If the marked position is on the same page as current position, we
	 * don't use markPos, but just keep the marked itemIndex in markItemIndex
	 * (all the rest of currPos is valid for the mark position). Hence, to
	 * determine if there is a mark, first look at markItemIndex, then at
	 * markPos.
	 */
	int			markItemIndex;	/* itemIndex, or -1 if not valid */

	/* keep these last in struct for efficiency */
	mbtScanPosData currPos;		/* current position data */
	mbtScanPosData markPos;		/* marked position, if any */
} mbtScanOpaqueData;

typedef mbtScanOpaqueData *mbtScanOpaque;

/*
 * We use some private sk_flags bits in preprocessed scan keys.  We're allowed
 * to use bits 16-31 (see skey.h).	The uppermost bits are copied from the
 * index's indoption[] array entry for the index attribute.
 */
#define SK_mbt_REQFWD	0x00010000		/* required to continue forward scan */
#define SK_mbt_REQBKWD	0x00020000		/* required to continue backward scan */
#define SK_mbt_INDOPTION_SHIFT  24		/* must clear the above bits */
#define SK_mbt_DESC			(INDOPTION_DESC << SK_mbt_INDOPTION_SHIFT)
#define SK_mbt_NULLS_FIRST	(INDOPTION_NULLS_FIRST << SK_mbt_INDOPTION_SHIFT)

/*
 * prototypes for functions in nmbtree.c (external entry points for mbtree)
 */
extern Datum mbtbuild(PG_FUNCTION_ARGS);
extern Datum mbtinsert(PG_FUNCTION_ARGS);
extern Datum mbtbeginscan(PG_FUNCTION_ARGS);
extern Datum mbtgettuple(PG_FUNCTION_ARGS);
extern Datum mbtgetbitmap(PG_FUNCTION_ARGS);
extern Datum mbtrescan(PG_FUNCTION_ARGS);
extern Datum mbtendscan(PG_FUNCTION_ARGS);
extern Datum mbtmarkpos(PG_FUNCTION_ARGS);
extern Datum mbtrestrpos(PG_FUNCTION_ARGS);
extern Datum mbtbulkdelete(PG_FUNCTION_ARGS);
extern Datum mbtvacuumcleanup(PG_FUNCTION_ARGS);
extern Datum mbtoptions(PG_FUNCTION_ARGS);

/*
 * prototypes for functions in nmbtinsert.c
 */
extern void _mbt_doinsert(Relation rel, IndexTuple itup,
			 bool index_is_unique, Relation heapRel);
extern Buffer _mbt_getstackbuf(Relation rel, mbtStack stack, int access);
extern void _mbt_insert_parent(Relation rel, Buffer buf, Buffer rbuf,
				  mbtStack stack, bool is_root, bool is_only);

/*
 * prototypes for functions in nmbtpage.c
 */
extern void _mbt_initmetapage(Page page, BlockNumber rootbknum, uint32 level);
extern Buffer _mbt_getroot(Relation rel, int access);
extern Buffer _mbt_gettrueroot(Relation rel);
extern void _mbt_checkpage(Relation rel, Buffer buf);
extern Buffer _mbt_getbuf(Relation rel, BlockNumber blkno, int access);
extern Buffer _mbt_relandgetbuf(Relation rel, Buffer obuf,
				 BlockNumber blkno, int access);
extern void _mbt_relbuf(Relation rel, Buffer buf);
extern void _mbt_pageinit(Page page, Size size);
extern bool _mbt_page_recyclable(Page page);
extern void _mbt_delitems(Relation rel, Buffer buf,
			 OffsetNumber *itemnos, int nitems);
extern int _mbt_pagedel(Relation rel, Buffer buf,
			mbtStack stack, bool vacuum_full);

/*
 * prototypes for functions in nmbtsearch.c
 */
extern mbtStack _mbt_search(Relation rel,
		   int keysz, ScanKey scankey, bool nextkey,
		   Buffer *bufP, int access);
extern Buffer _mbt_moveright(Relation rel, Buffer buf, int keysz,
			  ScanKey scankey, bool nextkey, int access);
extern OffsetNumber _mbt_binsrch(Relation rel, Buffer buf, int keysz,
			ScanKey scankey, bool nextkey);
extern int32 _mbt_compare(Relation rel, int keysz, ScanKey scankey,
			Page page, OffsetNumber offnum);
extern bool _mbt_first(IndexScanDesc scan, ScanDirection dir);
extern bool _mbt_next(IndexScanDesc scan, ScanDirection dir);
extern Buffer _mbt_get_endpoint(Relation rel, uint32 level, bool rightmost);

/*
 * prototypes for functions in nmbtutils.c
 */
extern ScanKey _mbt_mkscankey(Relation rel, IndexTuple itup);
extern ScanKey _mbt_mkscankey_nodata(Relation rel);
extern void _mbt_freeskey(ScanKey skey);
extern void _mbt_freestack(mbtStack stack);
extern void _mbt_preprocess_keys(IndexScanDesc scan);
extern bool _mbt_checkkeys(IndexScanDesc scan,
			  Page page, OffsetNumber offnum,
			  ScanDirection dir, bool *continuescan);
extern void _mbt_killitems(IndexScanDesc scan, bool haveLock);
extern mbtCycleId _mbt_vacuum_cycleid(Relation rel);
extern mbtCycleId _mbt_start_vacuum(Relation rel);
extern void _mbt_end_vacuum(Relation rel);
extern void _mbt_end_vacuum_callback(int code, Datum arg);
extern Size mbtreeShmemSize(void);
extern void mbtreeShmemInit(void);

/*
 * prototypes for functions in nmbtsort.c
 */
typedef struct mbtSpool mbtSpool; /* opaque type known only within nmbtsort.c */

extern mbtSpool *_mbt_spoolinit(Relation index, bool isunique, bool isdead);
extern void _mbt_spooldestroy(mbtSpool *mbtspool);
extern void _mbt_spool(IndexTuple itup, mbtSpool *mbtspool);
extern void _mbt_leafbuild(mbtSpool *mbtspool, mbtSpool *spool2);

/*
 * prototypes for functions in nmbtxlog.c
 */
extern void mbtree_redo(XLogRecPtr lsn, XLogRecord *record);
extern void mbtree_desc(StringInfo buf, uint8 xl_info, char *rec);
extern void mbtree_xlog_startup(void);
extern void mbtree_xlog_cleanup(void);
extern bool mbtree_safe_restartpoint(void);

#endif   /* NmbtREE_H */
