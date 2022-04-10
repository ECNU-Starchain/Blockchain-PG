/*-------------------------------------------------------------------------
 *
 * nodeIndexscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/nodeIndexscan.h,v 1.34 2009/01/01 17:23:59 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEINDEXSCAN_H
#define NODEINDEXSCAN_H

#include "nodes/execnodes.h"
//by zcy modified 2020/6/30
#define DEFAULTSIZE 8192
//modified end	

extern int	ExecCountSlotsIndexScan(IndexScan *node);
extern IndexScanState *ExecInitIndexScan(IndexScan *node, EState *estate, int eflags);
extern TupleTableSlot *ExecIndexScan(IndexScanState *node);



//added by gqx 2020/3/31
extern int ExecCountSlotsAuthIndexScan(IndexScan *node);




extern void ExecEndIndexScan(IndexScanState *node);
extern void ExecIndexMarkPos(IndexScanState *node);
extern void ExecIndexRestrPos(IndexScanState *node);
extern void ExecIndexReScan(IndexScanState *node, ExprContext *exprCtxt);

/* routines exported to share code with nodeBitmapIndexscan.c */
extern void ExecIndexBuildScanKeys(PlanState *planstate, Relation index,
					   Index scanrelid,
					   List *quals, ScanKey *scanKeys, int *numScanKeys,
					   IndexRuntimeKeyInfo **runtimeKeys, int *numRuntimeKeys,
					   IndexArrayKeyInfo **arrayKeys, int *numArrayKeys);
extern void ExecIndexEvalRuntimeKeys(ExprContext *econtext,
					   IndexRuntimeKeyInfo *runtimeKeys, int numRuntimeKeys);
extern bool ExecIndexEvalArrayKeys(ExprContext *econtext,
					   IndexArrayKeyInfo *arrayKeys, int numArrayKeys);
extern bool ExecIndexAdvanceArrayKeys(IndexArrayKeyInfo *arrayKeys, int numArrayKeys);


//added by gqx 2020/3/30
extern AuthIndexScanState *ExecInitAuthSeqScan(AuthIndexScan *node, EState *estate, int eflags);
//added by gqx 2020/3/30
extern TupleTableSlot *ExecAuthIndexScan(AuthIndexScanState *node);
//added by gqx 2020/3/31
extern void ExecEndAuthIndexScan(AuthIndexScanState *node);

#endif   /* NODEINDEXSCAN_H */
