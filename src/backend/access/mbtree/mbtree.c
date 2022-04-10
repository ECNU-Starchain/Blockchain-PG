/*-------------------------------------------------------------------------
 *
 * nmbtree.c
 *	  Implementation of Lehman and Yao's mbtree management algorithm for
 *	  Postgres.
 *
 * NOTES
 *	  This file contains only the public interface routines.
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/access/nmbtree/nmbtree.c,v 1.171 2009/06/11 14:48:54 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/mbtree.h"
#include "access/relscan.h"
#include "catalog/index.h"
#include "catalog/storage.h"
#include "commands/vacuum.h"
#include "storage/bufmgr.h"
#include "storage/freespace.h"
#include "storage/indexfsm.h"
#include "storage/ipc.h"
#include "storage/lmgr.h"
#include "utils/memutils.h"
#include <stdio.h>
#include "mbt/mbtree.h"
#include "mbt/testtree.h"
////////////////////////////////////////////////////////////////////////////
/* Working state for mbtbuild and its callback */
//int count1=0;
//double starttime=0;
//double endtime=0;
//double mytime=0;
//static inline double GetTime()
//{
//    struct timeval tv;
//    gettimeofday(&tv,NULL);
//    return tv.tv_sec + tv.tv_usec / 1e6;
//}

typedef struct
{
	bool		isUnique;
	bool		haveDead;
	Relation	heapRel;
	mbtSpool    *spool;

	/*
	 * spool2 is needed only when the index is an unique index. Dead tuples
	 * are put into spool2 instead of spool in order to avoid uniqueness
	 * check.
	 */
	mbtSpool    *spool2;
	double		indtuples;
} mbtBuildState;

/* Working state needed by mbtvacuumpage */
typedef struct
{
	IndexVacuumInfo *info;
	IndexBulkDeleteResult *stats;
	IndexBulkDeleteCallback callback;
	void	   *callback_state;
	mbtCycleId	cycleid;
	BlockNumber lastUsedPage;
	BlockNumber totFreePages;	/* true total # of free pages */
	MemoryContext pagedelcontext;
} mbtVacState;


static void mbtbuildCallback(Relation index,
				HeapTuple htup,
				Datum *values,
				bool *isnull,
				bool tupleIsAlive,
				void *state);
static void mbtvacuumscan(IndexVacuumInfo *info, IndexBulkDeleteResult *stats,
			 IndexBulkDeleteCallback callback, void *callback_state,
			 mbtCycleId cycleid);
static void mbtvacuumpage(mbtVacState *vstate, BlockNumber blkno,
			 BlockNumber orig_blkno);


Hashmap* key_hashp = NULL;
Hashmap* hashp[BPLUS_MAX_LEVEL];
int root_offset = 0;
int64_t temp_node_id[BPLUS_MAX_LEVEL][500] = {0};
int64_t mbt_entries_count = 0;
/*
 *	mbtbuild() -- build a new mbtree index.
 */
Datum
mbtbuild(PG_FUNCTION_ARGS)
{
 	Relation	heap = (Relation) PG_GETARG_POINTER(0);
 	Relation	index = (Relation) PG_GETARG_POINTER(1);
 	IndexInfo  *indexInfo = (IndexInfo *) PG_GETARG_POINTER(2);
 	IndexBuildResult *result;
 	double		reltuples;
    int i = 0;
    int index_oid = index->rd_node.relNode;

    if(key_hashp == NULL){
         key_hashp = hashmap_new(NUM_BUCKETS, myhash, compare);
    }

    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
        hashp[i] = hashmap_new(NUM_BUCKETS, myhash, compare);
    }


    bplus_tree* tree= bplus_tree_init(10,10);
    mybplus_tree_init(tree);
    hashmap_set(key_hashp, index_oid, tree);

    result = (IndexBuildResult *) palloc(sizeof(IndexBuildResult));

    result->heap_tuples = reltuples;
    result->index_tuples = 0;
    PG_RETURN_POINTER(result);
//    PG_RETURN_POINTER(result);
}

/*
 * Per-tuple callback from IndexBuildHeapScan
 */
static void
mbtbuildCallback(Relation index,
				HeapTuple htup,
				Datum *values,
				bool *isnull,
				bool tupleIsAlive,
				void *state)
{
	mbtBuildState *buildstate = (mbtBuildState *) state;
	IndexTuple	itup;

	/* form an index tuple and point it at the heap tuple */
	itup = index_form_tuple(RelationGetDescr(index), values, isnull);
	itup->t_tid = htup->t_self;

	/*
	 * insert the index tuple into the appropriate spool file for subsequent
	 * processing
	 */
	if (tupleIsAlive || buildstate->spool2 == NULL)
		_mbt_spool(itup, buildstate->spool);
	else
	{
		/* dead tuples are put into spool2 */
		buildstate->haveDead = true;
		_mbt_spool(itup, buildstate->spool2);
	}

	buildstate->indtuples += 1;

	pfree(itup);
}

/*
 *	mbtinsert() -- insert an index tuple into a mbtree.
 *
 *		Descend the tree recursively, find the appropriate location for our
 *		new tuple, and put it there.
 */
Datum
mbtinsert(PG_FUNCTION_ARGS)
{
    Relation	rel = (Relation) PG_GETARG_POINTER(0);
    pg_index_info *info = (pg_index_info *) PG_GETARG_POINTER(1);
    ItemPointer ht_ctid = (ItemPointer) PG_GETARG_POINTER(2);
    Relation	heapRel = (Relation) PG_GETARG_POINTER(3);
    ItemPointer tid = (ItemPointer)malloc(sizeof(ItemPointerData));
    tid->ip_blkid.bi_hi = ht_ctid->ip_blkid.bi_hi;
    tid->ip_blkid.bi_lo = ht_ctid->ip_blkid.bi_lo;
    tid->ip_posid = ht_ctid->ip_posid;
    bplus_tree* tree = (struct bplus_tree *)hashmap_get(key_hashp, rel->rd_node.relNode);
    bplus_tree_put(tree, info->value, info->str, ht_ctid);
    free(tid);
    //info->root_str = hashcompute((bplus_node *)tree->root);
    //set_node_id(tree,hashp,temp_node_id);
	//root_offset =StoreBtree(tree);
	//by zcy modified 2020/6/30
	mbt_entries_count++;
    if(mbt_entries_count>=998){
		info->root_str = hashcompute((bplus_node *)tree->root);
		set_node_id(tree,hashp,temp_node_id);
		root_offset =StoreBtree(tree);
	}
	//modifiend end
	printf("\nmbtinsert!!!\n");
    PG_RETURN_BOOL(true);
}

/*
 *	mbtgettuple() -- Get the next tuple in the scan.
 */
Datum
mbtgettuple(PG_FUNCTION_ARGS)
{
   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
   Relation indexRelation = (Relation) PG_GETARG_POINTER(1);
   int left = (int) PG_GETARG_INT32(2);
   int right = (int) PG_GETARG_INT32(3);
   int i = 0;
   pgvo vo =(rangevo*)malloc(sizeof(rangevo));
   //by zcy modified 2020/6/30
   int hashmap_size = 8192;
   int sizes = right - left +1;
   hashmap_size = (sizes/400+1)*hashmap_size;
//    switch(sizes/500)
//    {
// 	   case 0: break;
// 	   case 1: hashmap_size = hashmap_size*2; break;
// 	   case 2: hashmap_size = hashmap_size*3; break;
// 	   case 3: hashmap_size = hashmap_size*4; break;
// 	   case 4: hashmap_size = hashmap_size*5; break;
// 	   case 5: hashmap_size = hashmap_size*6; break;
// 	   case 6: hashmap_size = hashmap_size*7; break;
// 	   case 7: hashmap_size = hashmap_size*8; break;
// 	   case 8: hashmap_size = hashmap_size*9; break;
// 	   default: hashmap_size =hashmap_size*10; break;
//    }
   //modified end

   rangevo_init(vo,hashmap_size);
    bplus_tree* tree = (struct bplus_tree *)hashmap_get(key_hashp, indexRelation->rd_node.relNode);
//     int root_offset = *(int*)hashmap_get(offsethashp, indexRelation->rd_node.relNode);

    // char str[1024]; 
    hashmap_serialize(hashp, temp_node_id, vo->hashmap);

//    starttime=GetTime();
    AuthRangeQuery(tree, left, right, root_offset, vo);
//    endtime=GetTime();
//    mytime=mytime + ((double)(endtime-starttime));
//    count1++;
//    if(count1==99){
//        count1=0;
//        mytime=0;
//    }

//    char* buf=(char*)malloc(sizeof(char)*hashmap_size);
//    vo_serialize(vo,buf);


//    pgvo vo2 =(rangevo*)malloc(sizeof(rangevo));
//    rangevo_init(vo2,hashmap_size);
//    vo_deserialize(vo2, buf);

//    Hashmap* hashp2[BPLUS_MAX_LEVEL];
//    for(i = 0; i < BPLUS_MAX_LEVEL; i++){
//        hashp2[i] = hashmap_new(NUM_BUCKETS, myhash, compare);
//    }
//    hashmap_deserialize(hashp2, vo->hashmap);

//    //PG_RETURN_BOOL(res);
     char* root = (char*)malloc(33);
//      reconstruct_root(hashp2, *vo2, root);
// 	free(buf);
   //reconstruct_root(hashp, *vo, root);
    printf("\nmbtgettuple!!!\n");
    PG_RETURN_POINTER(vo);
}

/*
 * mbtgetbitmap() -- gets all matching tuples, and adds them to a bitmap
 */
Datum
mbtgetbitmap(PG_FUNCTION_ARGS)
{
	// IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	// TIDBitmap  *tbm = (TIDBitmap *) PG_GETARG_POINTER(1);
	// mbtScanOpaque so = (mbtScanOpaque) scan->opaque;
	// int64		ntids = 0;
	// ItemPointer heapTid;

	// /* Fetch the first page & tuple. */
	// if (!_mbt_first(scan, ForwardScanDirection))
	// {
	// 	/* embty scan */
	// 	PG_RETURN_INT64(0);
	// }
	// /* Save tuple ID, and continue scanning */
	// heapTid = &scan->xs_ctup.t_self;
	// tbm_add_tuples(tbm, heapTid, 1, false);
	// ntids++;

	// for (;;)
	// {
	// 	/*
	// 	 * Advance to next tuple within page.  This is the same as the easy
	// 	 * case in _mbt_next().
	// 	 */
	// 	if (++so->currPos.itemIndex > so->currPos.lastItem)
	// 	{
	// 		/* let _mbt_next do the heavy lifting */
	// 		if (!_mbt_next(scan, ForwardScanDirection))
	// 			break;
	// 	}

	// 	/* Save tuple ID, and continue scanning */
	// 	heapTid = &so->currPos.items[so->currPos.itemIndex].heapTid;
	// 	tbm_add_tuples(tbm, heapTid, 1, false);
	// 	ntids++;
	// }

	//PG_RETURN_INT64(ntids);
	printf("\nmbtgetbitmap!!!\n");
	PG_RETURN_INT64(0);
}

/*
 *	mbtbeginscan() -- start a scan on a mbtree index
 */
Datum
mbtbeginscan(PG_FUNCTION_ARGS)
{
	 Relation	rel = (Relation) PG_GETARG_POINTER(0);
	 int			keysz = PG_GETARG_INT32(1);
	 ScanKey		scankey = (ScanKey) PG_GETARG_POINTER(2);
	 IndexScanDesc scan;

	// /* get the scan */
	 scan = RelationGetIndexScan(rel, keysz, scankey);

	//PG_RETURN_POINTER(scan);
	printf("\nmbtbeginscan!!!\n");
	PG_RETURN_POINTER(scan);
}

/*
 *	mbtrescan() -- rescan an index relation
 */
Datum
mbtrescan(PG_FUNCTION_ARGS)
{
	// IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	// ScanKey		scankey = (ScanKey) PG_GETARG_POINTER(1);
	// mbtScanOpaque so;

	// so = (mbtScanOpaque) scan->opaque;

	// if (so == NULL)				/* if called from mbtbeginscan */
	// {
	// 	so = (mbtScanOpaque) palloc(sizeof(mbtScanOpaqueData));
	// 	so->currPos.buf = so->markPos.buf = InvalidBuffer;
	// 	if (scan->numberOfKeys > 0)
	// 		so->keyData = (ScanKey) palloc(scan->numberOfKeys * sizeof(ScanKeyData));
	// 	else
	// 		so->keyData = NULL;
	// 	so->killedItems = NULL; /* until needed */
	// 	so->numKilled = 0;
	// 	scan->opaque = so;
	// }

	// /* we aren't holding any read locks, but gotta drop the pins */
	// if (mbtScanPosIsValid(so->currPos))
	// {
	// 	/* Before leaving current page, deal with any killed items */
	// 	if (so->numKilled > 0)
	// 		_mbt_killitems(scan, false);
	// 	ReleaseBuffer(so->currPos.buf);
	// 	so->currPos.buf = InvalidBuffer;
	// }

	// if (mbtScanPosIsValid(so->markPos))
	// {
	// 	ReleaseBuffer(so->markPos.buf);
	// 	so->markPos.buf = InvalidBuffer;
	// }
	// so->markItemIndex = -1;

	// /*
	//  * Reset the scan keys. Note that keys ordering stuff moved to _mbt_first.
	//  * - vadim 05/05/97
	//  */
	// if (scankey && scan->numberOfKeys > 0)
	// 	memmove(scan->keyData,
	// 			scankey,
	// 			scan->numberOfKeys * sizeof(ScanKeyData));
	// so->numberOfKeys = 0;		/* until _mbt_preprocess_keys sets it */

	// PG_RETURN_VOID();
	printf("\nmbtrescan!!!\n");
	PG_RETURN_VOID();
}

/*
 *	mbtendscan() -- close down a scan
 */
Datum
mbtendscan(PG_FUNCTION_ARGS)
{
	// IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	// mbtScanOpaque so = (mbtScanOpaque) scan->opaque;

	// /* we aren't holding any read locks, but gotta drop the pins */
	// if (mbtScanPosIsValid(so->currPos))
	// {
	// 	/* Before leaving current page, deal with any killed items */
	// 	if (so->numKilled > 0)
	// 		_mbt_killitems(scan, false);
	// 	ReleaseBuffer(so->currPos.buf);
	// 	so->currPos.buf = InvalidBuffer;
	// }

	// if (mbtScanPosIsValid(so->markPos))
	// {
	// 	ReleaseBuffer(so->markPos.buf);
	// 	so->markPos.buf = InvalidBuffer;
	// }
	// so->markItemIndex = -1;

	// if (so->killedItems != NULL)
	// 	pfree(so->killedItems);
	// if (so->keyData != NULL)
	// 	pfree(so->keyData);
	// pfree(so);

	// PG_RETURN_VOID();
	printf("\nmbtendscan!!!\n");
	PG_RETURN_VOID();
}

/*
 *	mbtmarkpos() -- save current scan position
 */
Datum
mbtmarkpos(PG_FUNCTION_ARGS)
{
	// IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	// mbtScanOpaque so = (mbtScanOpaque) scan->opaque;

	// /* we aren't holding any read locks, but gotta drop the pin */
	// if (mbtScanPosIsValid(so->markPos))
	// {
	// 	ReleaseBuffer(so->markPos.buf);
	// 	so->markPos.buf = InvalidBuffer;
	// }

	// /*
	//  * Just record the current itemIndex.  If we later step to next page
	//  * before releasing the marked position, _mbt_steppage makes a full copy of
	//  * the currPos struct in markPos.  If (as often happens) the mark is moved
	//  * before we leave the page, we don't have to do that work.
	//  */
	// if (mbtScanPosIsValid(so->currPos))
	// 	so->markItemIndex = so->currPos.itemIndex;
	// else
	// 	so->markItemIndex = -1;

	// PG_RETURN_VOID();
	printf("\nmbtmarkpos!!!\n");
	PG_RETURN_VOID();
}

/*
 *	mbtrestrpos() -- restore scan to last saved position
 */
Datum
mbtrestrpos(PG_FUNCTION_ARGS)
{
	// IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	// mbtScanOpaque so = (mbtScanOpaque) scan->opaque;

	// if (so->markItemIndex >= 0)
	// {
	// 	/*
	// 	 * The mark position is on the same page we are currently on. Just
	// 	 * restore the itemIndex.
	// 	 */
	// 	so->currPos.itemIndex = so->markItemIndex;
	// }
	// else
	// {
	// 	/* we aren't holding any read locks, but gotta drop the pin */
	// 	if (mbtScanPosIsValid(so->currPos))
	// 	{
	// 		/* Before leaving current page, deal with any killed items */
	// 		if (so->numKilled > 0 &&
	// 			so->currPos.buf != so->markPos.buf)
	// 			_mbt_killitems(scan, false);
	// 		ReleaseBuffer(so->currPos.buf);
	// 		so->currPos.buf = InvalidBuffer;
	// 	}

	// 	if (mbtScanPosIsValid(so->markPos))
	// 	{
	// 		/* bump pin on mark buffer for assignment to current buffer */
	// 		IncrBufferRefCount(so->markPos.buf);
	// 		memcpy(&so->currPos, &so->markPos,
	// 			   offsetof(mbtScanPosData, items[1]) +
	// 			   so->markPos.lastItem * sizeof(mbtScanPosItem));
	// 	}
	// }
	// PG_RETURN_VOID();
	printf("\nmbtrestrpos!!!\n");
	PG_RETURN_VOID();
}

/*
 * Bulk deletion of all index entries pointing to a set of heap tuples.
 * The set of target tuples is specified via a callback routine that tells
 * whether any given heap tuple (identified by ItemPointer) is being deleted.
 *
 * Result: a palloc'd struct containing statistical info for VACUUM displays.
 */
Datum
mbtbulkdelete(PG_FUNCTION_ARGS)
{
	// IndexVacuumInfo *info = (IndexVacuumInfo *) PG_GETARG_POINTER(0);
	// IndexBulkDeleteResult *volatile stats = (IndexBulkDeleteResult *) PG_GETARG_POINTER(1);
	// IndexBulkDeleteCallback callback = (IndexBulkDeleteCallback) PG_GETARG_POINTER(2);
	// void	   *callback_state = (void *) PG_GETARG_POINTER(3);
	// Relation	rel = info->index;
	// mbtCycleId	cycleid;

	// /* allocate stats if first time through, else re-use existing struct */
	// if (stats == NULL)
	// 	stats = (IndexBulkDeleteResult *) palloc0(sizeof(IndexBulkDeleteResult));

	// /* Establish the vacuum cycle ID to use for this scan */
	// /* The ENSURE stuff ensures we clean up shared memory on failure */
	// PG_ENSURE_ERROR_CLEANUP(_mbt_end_vacuum_callback, PointerGetDatum(rel));
	// {
	// 	cycleid = _mbt_start_vacuum(rel);

	// 	mbtvacuumscan(info, stats, callback, callback_state, cycleid);
	// }
	// PG_END_ENSURE_ERROR_CLEANUP(_mbt_end_vacuum_callback, PointerGetDatum(rel));
	// _mbt_end_vacuum(rel);

	//PG_RETURN_POINTER(stats);
	printf("\nmbtbulkdelete!!!\n");
	PG_RETURN_POINTER(NULL);
}

/*
 * Post-VACUUM cleanup.
 *
 * Result: a palloc'd struct containing statistical info for VACUUM displays.
 */
Datum
mbtvacuumcleanup(PG_FUNCTION_ARGS)
{
	// IndexVacuumInfo *info = (IndexVacuumInfo *) PG_GETARG_POINTER(0);
	// IndexBulkDeleteResult *stats = (IndexBulkDeleteResult *) PG_GETARG_POINTER(1);

	// /* No-op in ANALYZE ONLY mode */
	// if (info->analyze_only)
	// 	PG_RETURN_POINTER(stats);

	// /*
	//  * If mbtbulkdelete was called, we need not do anything, just return the
	//  * stats from the latest mbtbulkdelete call.  If it wasn't called, we must
	//  * still do a pass over the index, to recycle any newly-recyclable pages
	//  * and to ombtain index statistics.
	//  *
	//  * Since we aren't going to actually delete any leaf items, there's no
	//  * need to go through all the vacuum-cycle-ID pushups.
	//  */
	// if (stats == NULL)
	// {
	// 	stats = (IndexBulkDeleteResult *) palloc0(sizeof(IndexBulkDeleteResult));
	// 	mbtvacuumscan(info, stats, NULL, NULL, 0);
	// }

	// /* Finally, vacuum the FSM */
	// IndexFreeSpaceMapVacuum(info->index);

	// /*
	//  * During a non-FULL vacuum it's quite possible for us to be fooled by
	//  * concurrent page splits into double-counting some index tuples, so
	//  * disbelieve any total that exceeds the underlying heap's count ... if we
	//  * know that accurately.  Otherwise this might just make matters worse.
	//  */
	// if (!info->vacuum_full && !info->estimated_count)
	// {
	// 	if (stats->num_index_tuples > info->num_heap_tuples)
	// 		stats->num_index_tuples = info->num_heap_tuples;
	// }

	// PG_RETURN_POINTER(stats);
	printf("\nmbtvacuumcleanup!!!\n");
	PG_RETURN_POINTER(NULL);
}

/*
 * mbtvacuumscan --- scan the index for VACUUMing purposes
 *
 * This combines the functions of looking for leaf tuples that are deletable
 * according to the vacuum callback, looking for embty pages that can be
 * deleted, and looking for old deleted pages that can be recycled.  Both
 * mbtbulkdelete and mbtvacuumcleanup invoke this (the latter only if no
 * mbtbulkdelete call occurred).
 *
 * The caller is responsible for initially allocating/zeroing a stats struct
 * and for ombtaining a vacuum cycle ID if necessary.
 */
static void
mbtvacuumscan(IndexVacuumInfo *info, IndexBulkDeleteResult *stats,
			 IndexBulkDeleteCallback callback, void *callback_state,
			 mbtCycleId cycleid)
{
	Relation	rel = info->index;
	mbtVacState	vstate;
	BlockNumber num_pages;
	BlockNumber blkno;
	bool		needLock;

	/*
	 * Reset counts that will be incremented during the scan; needed in case
	 * of multiple scans during a single VACUUM command
	 */
	stats->estimated_count = false;
	stats->num_index_tuples = 0;
	stats->pages_deleted = 0;

	/* Set up info to pass down to mbtvacuumpage */
	vstate.info = info;
	vstate.stats = stats;
	vstate.callback = callback;
	vstate.callback_state = callback_state;
	vstate.cycleid = cycleid;
	vstate.lastUsedPage = mbtREE_METAPAGE;
	vstate.totFreePages = 0;

	/* Create a temporary memory context to run _mbt_pagedel in */
	vstate.pagedelcontext = AllocSetContextCreate(CurrentMemoryContext,
												  "_mbt_pagedel",
												  ALLOCSET_DEFAULT_MINSIZE,
												  ALLOCSET_DEFAULT_INITSIZE,
												  ALLOCSET_DEFAULT_MAXSIZE);

	/*
	 * The outer loop iterates over all index pages except the metapage, in
	 * physical order (we hope the kernel will cooperate in providing
	 * read-ahead for speed).  It is critical that we visit all leaf pages,
	 * including ones added after we start the scan, else we might fail to
	 * delete some deletable tuples.  Hence, we must repeatedly check the
	 * relation length.  We must acquire the relation-extension lock while
	 * doing so to avoid a race condition: if someone else is extending the
	 * relation, there is a window where bufmgr/smgr have created a new
	 * all-zero page but it hasn't yet been write-locked by _mbt_getbuf(). If
	 * we manage to scan such a page here, we'll improperly assume it can be
	 * recycled.  Taking the lock synchronizes things enough to prevent a
	 * problem: either num_pages won't include the new page, or _mbt_getbuf
	 * already has write lock on the buffer and it will be fully initialized
	 * before we can examine it.  (See also vacuumlazy.c, which has the same
	 * issue.)	Also, we need not worry if a page is added immediately after
	 * we look; the page splitting code already has write-lock on the left
	 * page before it adds a right page, so we must already have processed any
	 * tuples due to be moved into such a page.
	 *
	 * We can skip locking for new or temp relations, however, since no one
	 * else could be accessing them.
	 */
	needLock = !RELATION_IS_LOCAL(rel);

	blkno = mbtREE_METAPAGE + 1;
	for (;;)
	{
		/* Get the current relation length */
		if (needLock)
			LockRelationForExtension(rel, ExclusiveLock);
		num_pages = RelationGetNumberOfBlocks(rel);
		if (needLock)
			UnlockRelationForExtension(rel, ExclusiveLock);

		/* Quit if we've scanned the whole relation */
		if (blkno >= num_pages)
			break;
		/* Iterate over pages, then loop back to recheck length */
		for (; blkno < num_pages; blkno++)
		{
			mbtvacuumpage(&vstate, blkno, blkno);
		}
	}

	/*
	 * During VACUUM FULL, we truncate off any recyclable pages at the end of
	 * the index.  In a normal vacuum it'd be unsafe to do this except by
	 * acquiring exclusive lock on the index and then rechecking all the
	 * pages; doesn't seem worth it.
	 */
	if (info->vacuum_full && vstate.lastUsedPage < num_pages - 1)
	{
		BlockNumber new_pages = vstate.lastUsedPage + 1;

		/*
		 * Okay to truncate.
		 */
		RelationTruncate(rel, new_pages);

		/* update statistics */
		stats->pages_removed += num_pages - new_pages;
		vstate.totFreePages -= (num_pages - new_pages);
		num_pages = new_pages;
	}

	MemoryContextDelete(vstate.pagedelcontext);

	/* update statistics */
	stats->num_pages = num_pages;
	stats->pages_free = vstate.totFreePages;
}

/*
 * mbtvacuumpage --- VACUUM one page
 *
 * This processes a single page for mbtvacuumscan().  In some cases we
 * must go back and re-examine previously-scanned pages; this routine
 * recurses when necessary to handle that case.
 *
 * blkno is the page to process.  orig_blkno is the highest block number
 * reached by the outer mbtvacuumscan loop (the same as blkno, unless we
 * are recursing to re-examine a previous page).
 */
static void
mbtvacuumpage(mbtVacState *vstate, BlockNumber blkno, BlockNumber orig_blkno)
{
	IndexVacuumInfo *info = vstate->info;
	IndexBulkDeleteResult *stats = vstate->stats;
	IndexBulkDeleteCallback callback = vstate->callback;
	void	   *callback_state = vstate->callback_state;
	Relation	rel = info->index;
	bool		delete_now;
	BlockNumber recurse_to;
	Buffer		buf;
	Page		page;
	mbtPageOpaque opaque;

restart:
	delete_now = false;
	recurse_to = P_NONE;

	/* call vacuum_delay_point while not holding any buffer lock */
	vacuum_delay_point();

	/*
	 * We can't use _mbt_getbuf() here because it always applies
	 * _mbt_checkpage(), which will barf on an all-zero page. We want to
	 * recycle all-zero pages, not fail.  Also, we want to use a nondefault
	 * buffer access strategy.
	 */
	buf = ReadBufferExtended(rel, MAIN_FORKNUM, blkno, RBM_NORMAL,
							 info->strategy);
	LockBuffer(buf, mbt_READ);
	page = BufferGetPage(buf);
	opaque = (mbtPageOpaque) PageGetSpecialPointer(page);
	if (!PageIsNew(page))
		_mbt_checkpage(rel, buf);

	/*
	 * If we are recursing, the only case we want to do anything with is a
	 * live leaf page having the current vacuum cycle ID.  Any other state
	 * implies we already saw the page (eg, deleted it as being embty).
	 */
	if (blkno != orig_blkno)
	{
		if (_mbt_page_recyclable(page) ||
			P_IGNORE(opaque) ||
			!P_ISLEAF(opaque) ||
			opaque->mbtpo_cycleid != vstate->cycleid)
		{
			_mbt_relbuf(rel, buf);
			return;
		}
	}

	/* If the page is in use, update lastUsedPage */
	if (!_mbt_page_recyclable(page) && vstate->lastUsedPage < blkno)
		vstate->lastUsedPage = blkno;

	/* Page is valid, see what to do with it */
	if (_mbt_page_recyclable(page))
	{
		/* Okay to recycle this page */
		RecordFreeIndexPage(rel, blkno);
		vstate->totFreePages++;
		stats->pages_deleted++;
	}
	else if (P_ISDELETED(opaque))
	{
		/* Already deleted, but can't recycle yet */
		stats->pages_deleted++;
	}
	else if (P_ISHALFDEAD(opaque))
	{
		/* Half-dead, try to delete */
		delete_now = true;
	}
	else if (P_ISLEAF(opaque))
	{
		OffsetNumber deletable[MaxOffsetNumber];
		int			ndeletable;
		OffsetNumber offnum,
					minoff,
					maxoff;

		/*
		 * Trade in the initial read lock for a super-exclusive write lock on
		 * this page.  We must get such a lock on every leaf page over the
		 * course of the vacuum scan, whether or not it actually contains any
		 * deletable tuples --- see nmbtree/README.
		 */
		LockBuffer(buf, BUFFER_LOCK_UNLOCK);
		LockBufferForCleanup(buf);

		/*
		 * Check whether we need to recurse back to earlier pages.	What we
		 * are concerned about is a page split that happened since we started
		 * the vacuum scan.  If the split moved some tuples to a lower page
		 * then we might have missed 'em.  If so, set up for tail recursion.
		 * (Must do this before possibly clearing mbtpo_cycleid below!)
		 */
		if (vstate->cycleid != 0 &&
			opaque->mbtpo_cycleid == vstate->cycleid &&
			!(opaque->mbtpo_flags & mbtP_SPLIT_END) &&
			!P_RIGHTMOST(opaque) &&
			opaque->mbtpo_next < orig_blkno)
			recurse_to = opaque->mbtpo_next;

		/*
		 * Scan over all items to see which ones need deleted according to the
		 * callback function.
		 */
		ndeletable = 0;
		minoff = P_FIRSTDATAKEY(opaque);
		maxoff = PageGetMaxOffsetNumber(page);
		if (callback)
		{
			for (offnum = minoff;
				 offnum <= maxoff;
				 offnum = OffsetNumberNext(offnum))
			{
				IndexTuple	itup;
				ItemPointer htup;

				itup = (IndexTuple) PageGetItem(page,
												PageGetItemId(page, offnum));
				htup = &(itup->t_tid);
				if (callback(htup, callback_state))
					deletable[ndeletable++] = offnum;
			}
		}

		/*
		 * Apply any needed deletes.  We issue just one _mbt_delitems() call
		 * per page, so as to minimize WAL traffic.
		 */
		if (ndeletable > 0)
		{
			_mbt_delitems(rel, buf, deletable, ndeletable);
			stats->tuples_removed += ndeletable;
			/* must recompute maxoff */
			maxoff = PageGetMaxOffsetNumber(page);
		}
		else
		{
			/*
			 * If the page has been split during this vacuum cycle, it seems
			 * worth expending a write to clear mbtpo_cycleid even if we don't
			 * have any deletions to do.  (If we do, _mbt_delitems takes care
			 * of this.)  This ensures we won't process the page again.
			 *
			 * We treat this like a hint-bit update because there's no need to
			 * WAL-log it.
			 */
			if (vstate->cycleid != 0 &&
				opaque->mbtpo_cycleid == vstate->cycleid)
			{
				opaque->mbtpo_cycleid = 0;
				SetBufferCommitInfoNeedsSave(buf);
			}
		}

		/*
		 * If it's now embty, try to delete; else count the live tuples. We
		 * don't delete when recursing, though, to avoid putting entries into
		 * freePages out-of-order (doesn't seem worth any extra code to handle
		 * the case).
		 */
		if (minoff > maxoff)
			delete_now = (blkno == orig_blkno);
		else
			stats->num_index_tuples += maxoff - minoff + 1;
	}

	if (delete_now)
	{
		MemoryContext oldcontext;
		int			ndel;

		/* Run pagedel in a temp context to avoid memory leakage */
		MemoryContextReset(vstate->pagedelcontext);
		oldcontext = MemoryContextSwitchTo(vstate->pagedelcontext);

		ndel = _mbt_pagedel(rel, buf, NULL, info->vacuum_full);

		/* count only this page, else may double-count parent */
		if (ndel)
			stats->pages_deleted++;

		/*
		 * During VACUUM FULL it's okay to recycle deleted pages immediately,
		 * since there can be no other transactions scanning the index.  Note
		 * that we will only recycle the current page and not any parent pages
		 * that _mbt_pagedel might have recursed to; this seems reasonable in
		 * the name of simplicity.	(Trying to do otherwise would mean we'd
		 * have to sort the list of recyclable pages we're building.)
		 */
		if (ndel && info->vacuum_full)
		{
			RecordFreeIndexPage(rel, blkno);
			vstate->totFreePages++;
		}

		MemoryContextSwitchTo(oldcontext);
		/* pagedel released buffer, so we shouldn't */
	}
	else
		_mbt_relbuf(rel, buf);

	/*
	 * This is really tail recursion, but if the compiler is too stupid to
	 * optimize it as such, we'd eat an uncomfortably large amount of stack
	 * space per recursion level (due to the deletable[] array). A failure is
	 * improbable since the number of levels isn't likely to be large ... but
	 * just in case, let's hand-optimize into a loop.
	 */
	if (recurse_to != P_NONE)
	{
		blkno = recurse_to;
		goto restart;
	}
}
