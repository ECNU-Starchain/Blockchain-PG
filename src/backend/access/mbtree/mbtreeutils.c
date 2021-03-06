/*-------------------------------------------------------------------------
 *
 * nmbtutils.c
 *	  Utility code for Postgres mbtree implementation.
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/access/nmbtree/nmbtutils.c,v 1.93 2009/01/05 17:14:28 alvherre Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <time.h>

#include "access/genam.h"
#include "access/mbtree.h"
#include "access/reloptions.h"
#include "access/relscan.h"
#include "executor/execdebug.h"
#include "miscadmin.h"
#include "storage/bufmgr.h"
#include "storage/lwlock.h"
#include "storage/shmem.h"
#include "utils/lsyscache.h"


static bool _mbt_compare_scankey_args(IndexScanDesc scan, ScanKey op,
						 ScanKey leftarg, ScanKey rightarg,
						 bool *result);
static void _mbt_mark_scankey_with_indoption(ScanKey skey, int16 *indoption);
static void _mbt_mark_scankey_required(ScanKey skey);
static bool _mbt_check_rowcompare(ScanKey skey,
					 IndexTuple tuple, TupleDesc tupdesc,
					 ScanDirection dir, bool *continuescan);


/*
 * _mbt_mkscankey
 *		Build an insertion scan key that contains comparison data from itup
 *		as well as comparator routines appropriate to the key datatypes.
 *
 *		The result is intended for use with _mbt_compare().
 */
ScanKey
_mbt_mkscankey(Relation rel, IndexTuple itup)
{
	ScanKey		skey;
	TupleDesc	itupdesc;
	int			natts;
	int16	   *indoption;
	int			i;

	itupdesc = RelationGetDescr(rel);
	natts = RelationGetNumberOfAttributes(rel);
	indoption = rel->rd_indoption;

	skey = (ScanKey) palloc(natts * sizeof(ScanKeyData));

	for (i = 0; i < natts; i++)
	{
		FmgrInfo   *procinfo;
		Datum		arg;
		bool		null;
		int			flags;

		/*
		 * We can use the cached (default) support procs since no cross-type
		 * comparison can be needed.
		 */
		procinfo = index_getprocinfo(rel, i + 1, mbtORDER_PROC);
		arg = index_getattr(itup, i + 1, itupdesc, &null);
		flags = (null ? SK_ISNULL : 0) | (indoption[i] << SK_mbt_INDOPTION_SHIFT);
		ScanKeyEntryInitializeWithInfo(&skey[i],
									   flags,
									   (AttrNumber) (i + 1),
									   InvalidStrategy,
									   InvalidOid,
									   procinfo,
									   arg);
	}

	return skey;
}

/*
 * _mbt_mkscankey_nodata
 *		Build an insertion scan key that contains 3-way comparator routines
 *		appropriate to the key datatypes, but no comparison data.  The
 *		comparison data ultimately used must match the key datatypes.
 *
 *		The result cannot be used with _mbt_compare(), unless comparison
 *		data is first stored into the key entries.	Currently this
 *		routine is only called by nmbtsort.c and tuplesort.c, which have
 *		their own comparison routines.
 */
ScanKey
_mbt_mkscankey_nodata(Relation rel)
{
	ScanKey		skey;
	int			natts;
	int16	   *indoption;
	int			i;

	natts = RelationGetNumberOfAttributes(rel);
	indoption = rel->rd_indoption;

	skey = (ScanKey) palloc(natts * sizeof(ScanKeyData));

	for (i = 0; i < natts; i++)
	{
		FmgrInfo   *procinfo;
		int			flags;

		/*
		 * We can use the cached (default) support procs since no cross-type
		 * comparison can be needed.
		 */
		procinfo = index_getprocinfo(rel, i + 1, mbtORDER_PROC);
		flags = SK_ISNULL | (indoption[i] << SK_mbt_INDOPTION_SHIFT);
		ScanKeyEntryInitializeWithInfo(&skey[i],
									   flags,
									   (AttrNumber) (i + 1),
									   InvalidStrategy,
									   InvalidOid,
									   procinfo,
									   (Datum) 0);
	}

	return skey;
}

/*
 * free a scan key made by either _mbt_mkscankey or _mbt_mkscankey_nodata.
 */
void
_mbt_freeskey(ScanKey skey)
{
	pfree(skey);
}

/*
 * free a retracement stack made by _mbt_search.
 */
void
_mbt_freestack(mbtStack stack)
{
	mbtStack		ostack;

	while (stack != NULL)
	{
		ostack = stack;
		stack = stack->mbts_parent;
		pfree(ostack);
	}
}


/*
 *	_mbt_preprocess_keys() -- Preprocess scan keys
 *
 * The caller-supplied search-type keys (in scan->keyData[]) are copied to
 * so->keyData[] with possible transformation.	scan->numberOfKeys is
 * the number of input keys, so->numberOfKeys gets the number of output
 * keys (possibly less, never greater).
 *
 * The output keys are marked with additional sk_flag bits beyond the
 * system-standard bits supplied by the caller.  The DESC and NULLS_FIRST
 * indoption bits for the relevant index attribute are copied into the flags.
 * Also, for a DESC column, we commute (flip) all the sk_strategy numbers
 * so that the index sorts in the desired direction.
 *
 * One key purpose of this routine is to discover how many scan keys
 * must be satisfied to continue the scan.	It also attembts to eliminate
 * redundant keys and detect contradictory keys.  (If the index opfamily
 * provides incomplete sets of cross-type operators, we may fail to detect
 * redundant or contradictory keys, but we can survive that.)
 *
 * The output keys must be sorted by index attribute.  Presently we expect
 * (but verify) that the input keys are already so sorted --- this is done
 * by group_clauses_by_indexkey() in indxpath.c.  Some reordering of the keys
 * within each attribute may be done as a byproduct of the processing here,
 * but no other code depends on that.
 *
 * The output keys are marked with flags SK_mbt_REQFWD and/or SK_mbt_REQBKWD
 * if they must be satisfied in order to continue the scan forward or backward
 * respectively.  _mbt_checkkeys uses these flags.  For example, if the quals
 * are "x = 1 AND y < 4 AND z < 5", then _mbt_checkkeys will reject a tuple
 * (1,2,7), but we must continue the scan in case there are tuples (1,3,z).
 * But once we reach tuples like (1,4,z) we can stop scanning because no
 * later tuples could match.  This is reflected by marking the x and y keys,
 * but not the z key, with SK_mbt_REQFWD.  In general, the keys for leading
 * attributes with "=" keys are marked both SK_mbt_REQFWD and SK_mbt_REQBKWD.
 * For the first attribute without an "=" key, any "<" and "<=" keys are
 * marked SK_mbt_REQFWD while any ">" and ">=" keys are marked SK_mbt_REQBKWD.
 * This can be seen to be correct by considering the above example.  Note
 * in particular that if there are no keys for a given attribute, the keys for
 * subsequent attributes can never be required; for instance "WHERE y = 4"
 * requires a full-index scan.
 *
 * If possible, redundant keys are eliminated: we keep only the tightest
 * >/>= bound and the tightest </<= bound, and if there's an = key then
 * that's the only one returned.  (So, we return either a single = key,
 * or one or two boundary-condition keys for each attr.)  However, if we
 * cannot compare two keys for lack of a suitable cross-type operator,
 * we cannot eliminate either.	If there are two such keys of the same
 * operator strategy, the second one is just pushed into the output array
 * without further processing here.  We may also emit both >/>= or both
 * </<= keys if we can't compare them.  The logic about required keys still
 * works if we don't eliminate redundant keys.
 *
 * As a byproduct of this work, we can detect contradictory quals such
 * as "x = 1 AND x > 2".  If we see that, we return so->qual_ok = FALSE,
 * indicating the scan need not be run at all since no tuples can match.
 * (In this case we do not bother completing the output key array!)
 * Again, missing cross-type operators might cause us to fail to prove the
 * quals contradictory when they really are, but the scan will work correctly.
 *
 * Row comparison keys are currently also treated without any smarts:
 * we just transfer them into the preprocessed array without any
 * editorialization.  We can treat them the same as an ordinary inequality
 * comparison on the row's first index column, for the purposes of the logic
 * about required keys.
 *
 * Note: the reason we have to copy the preprocessed scan keys into private
 * storage is that we are modifying the array based on comparisons of the
 * key argument values, which could change on a rescan.  Therefore we can't
 * overwrite the caller's data structure.
 */
void
_mbt_preprocess_keys(IndexScanDesc scan)
{
	mbtScanOpaque so = (mbtScanOpaque) scan->opaque;
	int			numberOfKeys = scan->numberOfKeys;
	int16	   *indoption = scan->indexRelation->rd_indoption;
	int			new_numberOfKeys;
	int			numberOfEqualCols;
	ScanKey		inkeys;
	ScanKey		outkeys;
	ScanKey		cur;
	ScanKey		xform[mbtMaxStrategyNumber];
	bool		test_result;
	int			i,
				j;
	AttrNumber	attno;

	/* initialize result variables */
	so->qual_ok = true;
	so->numberOfKeys = 0;

	if (numberOfKeys < 1)
		return;					/* done if qual-less scan */

	inkeys = scan->keyData;
	outkeys = so->keyData;
	cur = &inkeys[0];
	/* we check that input keys are correctly ordered */
	if (cur->sk_attno < 1)
		elog(ERROR, "mbtree index keys must be ordered by attribute");

	/* We can short-circuit most of the work if there's just one key */
	if (numberOfKeys == 1)
	{
		/*
		 * We treat all mbtree operators as strict (even if they're not so
		 * marked in pg_proc).	This means that it is impossible for an
		 * operator condition with a NULL comparison constant to succeed, and
		 * we can reject it right away.
		 *
		 * However, we now also support "x IS NULL" clauses as search
		 * conditions, so in that case keep going.	The planner has not filled
		 * in any particular strategy in this case, so set it to
		 * mbtEqualStrategyNumber --- we can treat IS NULL as an equality
		 * operator for purposes of search strategy.
		 */
		if (cur->sk_flags & SK_ISNULL)
		{
			if (cur->sk_flags & SK_SEARCHNULL)
			{
				cur->sk_strategy = mbtEqualStrategyNumber;
				cur->sk_subtype = InvalidOid;
			}
			else
				so->qual_ok = false;
		}
		_mbt_mark_scankey_with_indoption(cur, indoption);
		memcpy(outkeys, cur, sizeof(ScanKeyData));
		so->numberOfKeys = 1;
		/* We can mark the qual as required if it's for first index col */
		if (cur->sk_attno == 1)
			_mbt_mark_scankey_required(outkeys);
		return;
	}

	/*
	 * Otherwise, do the full set of pushups.
	 */
	new_numberOfKeys = 0;
	numberOfEqualCols = 0;

	/*
	 * Initialize for processing of keys for attr 1.
	 *
	 * xform[i] points to the currently best scan key of strategy type i+1; it
	 * is NULL if we haven't yet found such a key for this attr.
	 */
	attno = 1;
	memset(xform, 0, sizeof(xform));

	/*
	 * Loop iterates from 0 to numberOfKeys inclusive; we use the last pass to
	 * handle after-last-key processing.  Actual exit from the loop is at the
	 * "break" statement below.
	 */
	for (i = 0;; cur++, i++)
	{
		if (i < numberOfKeys)
		{
			/* See comments above about NULLs and IS NULL handling. */
			/* Note: we assume SK_ISNULL is never set in a row header key */
			if (cur->sk_flags & SK_ISNULL)
			{
				if (cur->sk_flags & SK_SEARCHNULL)
				{
					cur->sk_strategy = mbtEqualStrategyNumber;
					cur->sk_subtype = InvalidOid;
				}
				else
				{
					so->qual_ok = false;
					return;
				}
			}
		}

		/*
		 * If we are at the end of the keys for a particular attr, finish up
		 * processing and emit the cleaned-up keys.
		 */
		if (i == numberOfKeys || cur->sk_attno != attno)
		{
			int			priorNumberOfEqualCols = numberOfEqualCols;

			/* check input keys are correctly ordered */
			if (i < numberOfKeys && cur->sk_attno < attno)
				elog(ERROR, "mbtree index keys must be ordered by attribute");

			/*
			 * If = has been specified, all other keys can be eliminated as
			 * redundant.  In case of key > 2 && key == 1 we can set qual_ok
			 * to false and abandon further processing.
			 */
			if (xform[mbtEqualStrategyNumber - 1])
			{
				ScanKey		eq = xform[mbtEqualStrategyNumber - 1];

				for (j = mbtMaxStrategyNumber; --j >= 0;)
				{
					ScanKey		chk = xform[j];

					if (!chk || j == (mbtEqualStrategyNumber - 1))
						continue;

					/* IS NULL together with any other predicate must fail */
					if (eq->sk_flags & SK_SEARCHNULL)
					{
						so->qual_ok = false;
						return;
					}

					if (_mbt_compare_scankey_args(scan, chk, eq, chk,
												 &test_result))
					{
						if (!test_result)
						{
							/* keys proven mutually contradictory */
							so->qual_ok = false;
							return;
						}
						/* else discard the redundant non-equality key */
						xform[j] = NULL;
					}
					/* else, cannot determine redundancy, keep both keys */
				}
				/* track number of attrs for which we have "=" keys */
				numberOfEqualCols++;
			}

			/* try to keep only one of <, <= */
			if (xform[mbtLessStrategyNumber - 1]
				&& xform[mbtLessEqualStrategyNumber - 1])
			{
				ScanKey		lt = xform[mbtLessStrategyNumber - 1];
				ScanKey		le = xform[mbtLessEqualStrategyNumber - 1];

				if (_mbt_compare_scankey_args(scan, le, lt, le,
											 &test_result))
				{
					if (test_result)
						xform[mbtLessEqualStrategyNumber - 1] = NULL;
					else
						xform[mbtLessStrategyNumber - 1] = NULL;
				}
			}

			/* try to keep only one of >, >= */
			if (xform[mbtGreaterStrategyNumber - 1]
				&& xform[mbtGreaterEqualStrategyNumber - 1])
			{
				ScanKey		gt = xform[mbtGreaterStrategyNumber - 1];
				ScanKey		ge = xform[mbtGreaterEqualStrategyNumber - 1];

				if (_mbt_compare_scankey_args(scan, ge, gt, ge,
											 &test_result))
				{
					if (test_result)
						xform[mbtGreaterEqualStrategyNumber - 1] = NULL;
					else
						xform[mbtGreaterStrategyNumber - 1] = NULL;
				}
			}

			/*
			 * Emit the cleaned-up keys into the outkeys[] array, and then
			 * mark them if they are required.	They are required (possibly
			 * only in one direction) if all attrs before this one had "=".
			 */
			for (j = mbtMaxStrategyNumber; --j >= 0;)
			{
				if (xform[j])
				{
					ScanKey		outkey = &outkeys[new_numberOfKeys++];

					memcpy(outkey, xform[j], sizeof(ScanKeyData));
					if (priorNumberOfEqualCols == attno - 1)
						_mbt_mark_scankey_required(outkey);
				}
			}

			/*
			 * Exit loop here if done.
			 */
			if (i == numberOfKeys)
				break;

			/* Re-initialize for new attno */
			attno = cur->sk_attno;
			memset(xform, 0, sizeof(xform));
		}

		/* apply indoption to scankey (might change sk_strategy!) */
		_mbt_mark_scankey_with_indoption(cur, indoption);

		/* check strategy this key's operator corresponds to */
		j = cur->sk_strategy - 1;

		/* if row comparison, push it directly to the output array */
		if (cur->sk_flags & SK_ROW_HEADER)
		{
			ScanKey		outkey = &outkeys[new_numberOfKeys++];

			memcpy(outkey, cur, sizeof(ScanKeyData));
			if (numberOfEqualCols == attno - 1)
				_mbt_mark_scankey_required(outkey);

			/*
			 * We don't support RowCompare using equality; such a qual would
			 * mess up the numberOfEqualCols tracking.
			 */
			Assert(j != (mbtEqualStrategyNumber - 1));
			continue;
		}

		/* have we seen one of these before? */
		if (xform[j] == NULL)
		{
			/* nope, so remember this scankey */
			xform[j] = cur;
		}
		else
		{
			/* yup, keep only the more restrictive key */

			/* if either arg is NULL, don't try to compare */
			if ((cur->sk_flags | xform[j]->sk_flags) & SK_ISNULL)
			{
				/* at least one of them must be an IS NULL clause */
				Assert(j == (mbtEqualStrategyNumber - 1));
				Assert((cur->sk_flags | xform[j]->sk_flags) & SK_SEARCHNULL);
				/* if one is and one isn't, the search must fail */
				if ((cur->sk_flags ^ xform[j]->sk_flags) & SK_SEARCHNULL)
				{
					so->qual_ok = false;
					return;
				}
				/* we have duplicate IS NULL clauses, ignore the newer one */
				continue;
			}

			if (_mbt_compare_scankey_args(scan, cur, cur, xform[j],
										 &test_result))
			{
				if (test_result)
					xform[j] = cur;
				else if (j == (mbtEqualStrategyNumber - 1))
				{
					/* key == a && key == b, but a != b */
					so->qual_ok = false;
					return;
				}
				/* else old key is more restrictive, keep it */
			}
			else
			{
				/*
				 * We can't determine which key is more restrictive.  Keep the
				 * previous one in xform[j] and push this one directly to the
				 * output array.
				 */
				ScanKey		outkey = &outkeys[new_numberOfKeys++];

				memcpy(outkey, cur, sizeof(ScanKeyData));
				if (numberOfEqualCols == attno - 1)
					_mbt_mark_scankey_required(outkey);
			}
		}
	}

	so->numberOfKeys = new_numberOfKeys;
}

/*
 * Compare two scankey values using a specified operator.  Both values
 * must be already known non-NULL.
 *
 * The test we want to perform is logically "leftarg op rightarg", where
 * leftarg and rightarg are the sk_argument values in those ScanKeys, and
 * the comparison operator is the one in the op ScanKey.  However, in
 * cross-data-type situations we may need to look up the correct operator in
 * the index's opfamily: it is the one having amopstrategy = op->sk_strategy
 * and amoplefttype/amoprighttype equal to the two argument datatypes.
 *
 * If the opfamily doesn't supply a complete set of cross-type operators we
 * may not be able to make the comparison.	If we can make the comparison
 * we store the operator result in *result and return TRUE.  We return FALSE
 * if the comparison could not be made.
 *
 * Note: op always points at the same ScanKey as either leftarg or rightarg.
 * Since we don't scribble on the scankeys, this aliasing should cause no
 * trouble.
 *
 * Note: this routine needs to be insensitive to any DESC option applied
 * to the index column.  For example, "x < 4" is a tighter constraint than
 * "x < 5" regardless of which way the index is sorted.  We don't worry about
 * NULLS FIRST/LAST either, since the given values are never nulls.
 */
static bool
_mbt_compare_scankey_args(IndexScanDesc scan, ScanKey op,
						 ScanKey leftarg, ScanKey rightarg,
						 bool *result)
{
	Relation	rel = scan->indexRelation;
	Oid			lefttype,
				righttype,
				optype,
				opcintype,
				cmp_op;
	StrategyNumber strat;

	/*
	 * The opfamily we need to worry about is identified by the index column.
	 */
	Assert(leftarg->sk_attno == rightarg->sk_attno);

	opcintype = rel->rd_opcintype[leftarg->sk_attno - 1];

	/*
	 * Determine the actual datatypes of the ScanKey arguments.  We have to
	 * support the convention that sk_subtype == InvalidOid means the opclass
	 * input type; this is a hack to simplify life for ScanKeyInit().
	 */
	lefttype = leftarg->sk_subtype;
	if (lefttype == InvalidOid)
		lefttype = opcintype;
	righttype = rightarg->sk_subtype;
	if (righttype == InvalidOid)
		righttype = opcintype;
	optype = op->sk_subtype;
	if (optype == InvalidOid)
		optype = opcintype;

	/*
	 * If leftarg and rightarg match the types expected for the "op" scankey,
	 * we can use its already-looked-up comparison function.
	 */
	if (lefttype == opcintype && righttype == optype)
	{
		*result = DatumGetBool(FunctionCall2(&op->sk_func,
											 leftarg->sk_argument,
											 rightarg->sk_argument));
		return true;
	}

	/*
	 * Otherwise, we need to go to the syscache to find the appropriate
	 * operator.  (This cannot result in infinite recursion, since no
	 * indexscan initiated by syscache lookup will use cross-data-type
	 * operators.)
	 *
	 * If the sk_strategy was flipped by _mbt_mark_scankey_with_indoption, we
	 * have to un-flip it to get the correct opfamily member.
	 */
	strat = op->sk_strategy;
	if (op->sk_flags & SK_mbt_DESC)
		strat = mbtCommuteStrategyNumber(strat);

	cmp_op = get_opfamily_member(rel->rd_opfamily[leftarg->sk_attno - 1],
								 lefttype,
								 righttype,
								 strat);
	if (OidIsValid(cmp_op))
	{
		RegProcedure cmp_proc = get_opcode(cmp_op);

		if (RegProcedureIsValid(cmp_proc))
		{
			*result = DatumGetBool(OidFunctionCall2(cmp_proc,
													leftarg->sk_argument,
													rightarg->sk_argument));
			return true;
		}
	}

	/* Can't make the comparison */
	*result = false;			/* suppress compiler warnings */
	return false;
}

/*
 * Mark a scankey with info from the index's indoption array.
 *
 * We copy the appropriate indoption value into the scankey sk_flags
 * (shifting to avoid clobbering system-defined flag bits).  Also, if
 * the DESC option is set, commute (flip) the operator strategy number.
 *
 * This function is applied to the *input* scankey structure; therefore
 * on a rescan we will be looking at already-processed scankeys.  Hence
 * we have to be careful not to re-commute the strategy if we already did it.
 * It's a bit ugly to modify the caller's copy of the scankey but in practice
 * there shouldn't be any problem, since the index's indoptions are certainly
 * not going to change while the scankey survives.
 */
static void
_mbt_mark_scankey_with_indoption(ScanKey skey, int16 *indoption)
{
	int			addflags;

	addflags = indoption[skey->sk_attno - 1] << SK_mbt_INDOPTION_SHIFT;
	if ((addflags & SK_mbt_DESC) && !(skey->sk_flags & SK_mbt_DESC))
		skey->sk_strategy = mbtCommuteStrategyNumber(skey->sk_strategy);
	skey->sk_flags |= addflags;

	if (skey->sk_flags & SK_ROW_HEADER)
	{
		ScanKey		subkey = (ScanKey) DatumGetPointer(skey->sk_argument);

		for (;;)
		{
			Assert(subkey->sk_flags & SK_ROW_MEMBER);
			addflags = indoption[subkey->sk_attno - 1] << SK_mbt_INDOPTION_SHIFT;
			if ((addflags & SK_mbt_DESC) && !(subkey->sk_flags & SK_mbt_DESC))
				subkey->sk_strategy = mbtCommuteStrategyNumber(subkey->sk_strategy);
			subkey->sk_flags |= addflags;
			if (subkey->sk_flags & SK_ROW_END)
				break;
			subkey++;
		}
	}
}

/*
 * Mark a scankey as "required to continue the scan".
 *
 * Depending on the operator type, the key may be required for both scan
 * directions or just one.	Also, if the key is a row comparison header,
 * we have to mark the appropriate subsidiary ScanKeys as required.  In
 * such cases, the first subsidiary key is required, but subsequent ones
 * are required only as long as they correspond to successive index columns
 * and match the leading column as to sort direction.
 * Otherwise the row comparison ordering is different from the index ordering
 * and so we can't stop the scan on the basis of those lower-order columns.
 *
 * Note: when we set required-key flag bits in a subsidiary scankey, we are
 * scribbling on a data structure belonging to the index AM's caller, not on
 * our private copy.  This should be OK because the marking will not change
 * from scan to scan within a query, and so we'd just re-mark the same way
 * anyway on a rescan.	Something to keep an eye on though.
 */
static void
_mbt_mark_scankey_required(ScanKey skey)
{
	int			addflags;

	switch (skey->sk_strategy)
	{
		case BTLessStrategyNumber:
		case BTLessEqualStrategyNumber:
		case mbtLessStrategyNumber:
		case mbtLessEqualStrategyNumber:
			addflags = SK_mbt_REQFWD;
			break;
		case BTEqualStrategyNumber:		
		case mbtEqualStrategyNumber:
			addflags = SK_mbt_REQFWD | SK_mbt_REQBKWD;
			break;
		case BTGreaterEqualStrategyNumber:
		case BTGreaterStrategyNumber:			
		case mbtGreaterEqualStrategyNumber:
		case mbtGreaterStrategyNumber:
			addflags = SK_mbt_REQBKWD;
			break;
		default:
			elog(ERROR, "unrecognized StrategyNumber: %d",
				 (int) skey->sk_strategy);
			addflags = 0;		/* keep compiler quiet */
			break;
	}

	skey->sk_flags |= addflags;

	if (skey->sk_flags & SK_ROW_HEADER)
	{
		ScanKey		subkey = (ScanKey) DatumGetPointer(skey->sk_argument);
		AttrNumber	attno = skey->sk_attno;

		/* First subkey should be same as the header says */
		Assert(subkey->sk_attno == attno);

		for (;;)
		{
			Assert(subkey->sk_flags & SK_ROW_MEMBER);
			if (subkey->sk_attno != attno)
				break;			/* non-adjacent key, so not required */
			if (subkey->sk_strategy != skey->sk_strategy)
				break;			/* wrong direction, so not required */
			subkey->sk_flags |= addflags;
			if (subkey->sk_flags & SK_ROW_END)
				break;
			subkey++;
			attno++;
		}
	}
}

/*
 * Test whether an indextuple satisfies all the scankey conditions.
 *
 * If so, copy its TID into scan->xs_ctup.t_self, and return TRUE.
 * If not, return FALSE (xs_ctup is not changed).
 *
 * If the tuple fails to pass the qual, we also determine whether there's
 * any need to continue the scan beyond this tuple, and set *continuescan
 * accordingly.  See comments for _mbt_preprocess_keys(), above, about how
 * this is done.
 *
 * scan: index scan descriptor (containing a search-type scankey)
 * page: buffer page containing index tuple
 * offnum: offset number of index tuple (must be a valid item!)
 * dir: direction we are scanning in
 * continuescan: output parameter (will be set correctly in all cases)
 */
bool
_mbt_checkkeys(IndexScanDesc scan,
			  Page page, OffsetNumber offnum,
			  ScanDirection dir, bool *continuescan)
{
	ItemId		iid = PageGetItemId(page, offnum);
	bool		tuple_valid;
	IndexTuple	tuple;
	TupleDesc	tupdesc;
	mbtScanOpaque so;
	int			keysz;
	int			ikey;
	ScanKey		key;

	*continuescan = true;		/* default assumbtion */

	/*
	 * If the scan specifies not to return killed tuples, then we treat a
	 * killed tuple as not passing the qual.  Most of the time, it's a win to
	 * not bother examining the tuple's index keys, but just return
	 * immediately with continuescan = true to proceed to the next tuple.
	 * However, if this is the last tuple on the page, we should check the
	 * index keys to prevent uselessly advancing to the next page.
	 */
	if (scan->ignore_killed_tuples && ItemIdIsDead(iid))
	{
		/* return immediately if there are more tuples on the page */
		if (ScanDirectionIsForward(dir))
		{
			if (offnum < PageGetMaxOffsetNumber(page))
				return false;
		}
		else
		{
			mbtPageOpaque opaque = (mbtPageOpaque) PageGetSpecialPointer(page);

			if (offnum > P_FIRSTDATAKEY(opaque))
				return false;
		}

		/*
		 * OK, we want to check the keys, but we'll return FALSE even if the
		 * tuple passes the key tests.
		 */
		tuple_valid = false;
	}
	else
		tuple_valid = true;

	tuple = (IndexTuple) PageGetItem(page, iid);

	IncrIndexProcessed();

	tupdesc = RelationGetDescr(scan->indexRelation);
	so = (mbtScanOpaque) scan->opaque;
	keysz = so->numberOfKeys;

	for (key = so->keyData, ikey = 0; ikey < keysz; key++, ikey++)
	{
		Datum		datum;
		bool		isNull;
		Datum		test;

		/* row-comparison keys need special processing */
		if (key->sk_flags & SK_ROW_HEADER)
		{
			if (_mbt_check_rowcompare(key, tuple, tupdesc, dir, continuescan))
				continue;
			return false;
		}

		datum = index_getattr(tuple,
							  key->sk_attno,
							  tupdesc,
							  &isNull);

		if (key->sk_flags & SK_ISNULL)
		{
			/* Handle IS NULL tests */
			Assert(key->sk_flags & SK_SEARCHNULL);

			if (isNull)
				continue;		/* tuple satisfies this qual */

			/*
			 * Tuple fails this qual.  If it's a required qual for the current
			 * scan direction, then we can conclude no further tuples will
			 * pass, either.
			 */
			if ((key->sk_flags & SK_mbt_REQFWD) &&
				ScanDirectionIsForward(dir))
				*continuescan = false;
			else if ((key->sk_flags & SK_mbt_REQBKWD) &&
					 ScanDirectionIsBackward(dir))
				*continuescan = false;

			/*
			 * In any case, this indextuple doesn't match the qual.
			 */
			return false;
		}

		if (isNull)
		{
			if (key->sk_flags & SK_mbt_NULLS_FIRST)
			{
				/*
				 * Since NULLs are sorted before non-NULLs, we know we have
				 * reached the lower limit of the range of values for this
				 * index attr.	On a backward scan, we can stop if this qual
				 * is one of the "must match" subset.  On a forward scan,
				 * however, we should keep going.
				 */
				if ((key->sk_flags & SK_mbt_REQBKWD) &&
					ScanDirectionIsBackward(dir))
					*continuescan = false;
			}
			else
			{
				/*
				 * Since NULLs are sorted after non-NULLs, we know we have
				 * reached the upper limit of the range of values for this
				 * index attr.	On a forward scan, we can stop if this qual is
				 * one of the "must match" subset.	On a backward scan,
				 * however, we should keep going.
				 */
				if ((key->sk_flags & SK_mbt_REQFWD) &&
					ScanDirectionIsForward(dir))
					*continuescan = false;
			}

			/*
			 * In any case, this indextuple doesn't match the qual.
			 */
			return false;
		}

		test = FunctionCall2(&key->sk_func, datum, key->sk_argument);

		if (!DatumGetBool(test))
		{
			/*
			 * Tuple fails this qual.  If it's a required qual for the current
			 * scan direction, then we can conclude no further tuples will
			 * pass, either.
			 *
			 * Note: because we stop the scan as soon as any required equality
			 * qual fails, it is critical that equality quals be used for the
			 * initial positioning in _mbt_first() when they are available. See
			 * comments in _mbt_first().
			 */
			if ((key->sk_flags & SK_mbt_REQFWD) &&
				ScanDirectionIsForward(dir))
				*continuescan = false;
			else if ((key->sk_flags & SK_mbt_REQBKWD) &&
					 ScanDirectionIsBackward(dir))
				*continuescan = false;

			/*
			 * In any case, this indextuple doesn't match the qual.
			 */
			return false;
		}
	}

	/* If we get here, the tuple passes all index quals. */
	if (tuple_valid)
		scan->xs_ctup.t_self = tuple->t_tid;

	return tuple_valid;
}

/*
 * Test whether an indextuple satisfies a row-comparison scan condition.
 *
 * Return true if so, false if not.  If not, also clear *continuescan if
 * it's not possible for any future tuples in the current scan direction
 * to pass the qual.
 *
 * This is a subroutine for _mbt_checkkeys, which see for more info.
 */
static bool
_mbt_check_rowcompare(ScanKey skey, IndexTuple tuple, TupleDesc tupdesc,
					 ScanDirection dir, bool *continuescan)
{
	ScanKey		subkey = (ScanKey) DatumGetPointer(skey->sk_argument);
	int32		cmpresult = 0;
	bool		result;

	/* First subkey should be same as the header says */
	Assert(subkey->sk_attno == skey->sk_attno);

	/* Loop over columns of the row condition */
	for (;;)
	{
		Datum		datum;
		bool		isNull;

		Assert(subkey->sk_flags & SK_ROW_MEMBER);

		datum = index_getattr(tuple,
							  subkey->sk_attno,
							  tupdesc,
							  &isNull);

		if (isNull)
		{
			if (subkey->sk_flags & SK_mbt_NULLS_FIRST)
			{
				/*
				 * Since NULLs are sorted before non-NULLs, we know we have
				 * reached the lower limit of the range of values for this
				 * index attr. On a backward scan, we can stop if this qual is
				 * one of the "must match" subset.	On a forward scan,
				 * however, we should keep going.
				 */
				if ((subkey->sk_flags & SK_mbt_REQBKWD) &&
					ScanDirectionIsBackward(dir))
					*continuescan = false;
			}
			else
			{
				/*
				 * Since NULLs are sorted after non-NULLs, we know we have
				 * reached the upper limit of the range of values for this
				 * index attr. On a forward scan, we can stop if this qual is
				 * one of the "must match" subset.	On a backward scan,
				 * however, we should keep going.
				 */
				if ((subkey->sk_flags & SK_mbt_REQFWD) &&
					ScanDirectionIsForward(dir))
					*continuescan = false;
			}

			/*
			 * In any case, this indextuple doesn't match the qual.
			 */
			return false;
		}

		if (subkey->sk_flags & SK_ISNULL)
		{
			/*
			 * Unlike the simple-scankey case, this isn't a disallowed case.
			 * But it can never match.	If all the earlier row comparison
			 * columns are required for the scan direction, we can stop the
			 * scan, because there can't be another tuple that will succeed.
			 */
			if (subkey != (ScanKey) DatumGetPointer(skey->sk_argument))
				subkey--;
			if ((subkey->sk_flags & SK_mbt_REQFWD) &&
				ScanDirectionIsForward(dir))
				*continuescan = false;
			else if ((subkey->sk_flags & SK_mbt_REQBKWD) &&
					 ScanDirectionIsBackward(dir))
				*continuescan = false;
			return false;
		}

		/* Perform the test --- three-way comparison not bool operator */
		cmpresult = DatumGetInt32(FunctionCall2(&subkey->sk_func,
												datum,
												subkey->sk_argument));

		if (subkey->sk_flags & SK_mbt_DESC)
			cmpresult = -cmpresult;

		/* Done comparing if unequal, else advance to next column */
		if (cmpresult != 0)
			break;

		if (subkey->sk_flags & SK_ROW_END)
			break;
		subkey++;
	}

	/*
	 * At this point cmpresult indicates the overall result of the row
	 * comparison, and subkey points to the deciding column (or the last
	 * column if the result is "=").
	 */
	switch (subkey->sk_strategy)
	{
			/* EQ and NE cases aren't allowed here */
		case mbtLessStrategyNumber:
			result = (cmpresult < 0);
			break;
		case mbtLessEqualStrategyNumber:
			result = (cmpresult <= 0);
			break;
		case mbtGreaterEqualStrategyNumber:
			result = (cmpresult >= 0);
			break;
		case mbtGreaterStrategyNumber:
			result = (cmpresult > 0);
			break;
		default:
			elog(ERROR, "unrecognized RowCompareType: %d",
				 (int) subkey->sk_strategy);
			result = 0;			/* keep compiler quiet */
			break;
	}

	if (!result)
	{
		/*
		 * Tuple fails this qual.  If it's a required qual for the current
		 * scan direction, then we can conclude no further tuples will pass,
		 * either.	Note we have to look at the deciding column, not
		 * necessarily the first or last column of the row condition.
		 */
		if ((subkey->sk_flags & SK_mbt_REQFWD) &&
			ScanDirectionIsForward(dir))
			*continuescan = false;
		else if ((subkey->sk_flags & SK_mbt_REQBKWD) &&
				 ScanDirectionIsBackward(dir))
			*continuescan = false;
	}

	return result;
}

/*
 * _mbt_killitems - set LP_DEAD state for items an indexscan caller has
 * told us were killed
 *
 * scan->so contains information about the current page and killed tuples
 * thereon (generally, this should only be called if so->numKilled > 0).
 *
 * The caller must have pin on so->currPos.buf, but may or may not have
 * read-lock, as indicated by haveLock.  Note that we assume read-lock
 * is sufficient for setting LP_DEAD status (which is only a hint).
 *
 * We match items by heap TID before assuming they are the right ones to
 * delete.	We cope with cases where items have moved right due to insertions.
 * If an item has moved off the current page due to a split, we'll fail to
 * find it and do nothing (this is not an error case --- we assume the item
 * will eventually get marked in a future indexscan).  Note that because we
 * hold pin on the target page continuously from initially reading the items
 * until applying this function, VACUUM cannot have deleted any items from
 * the page, and so there is no need to search left from the recorded offset.
 * (This observation also guarantees that the item is still the right one
 * to delete, which might otherwise be questionable since heap TIDs can get
 * recycled.)
 */
void
_mbt_killitems(IndexScanDesc scan, bool haveLock)
{
	mbtScanOpaque so = (mbtScanOpaque) scan->opaque;
	Page		page;
	mbtPageOpaque opaque;
	OffsetNumber minoff;
	OffsetNumber maxoff;
	int			i;
	bool		killedsomething = false;

	Assert(BufferIsValid(so->currPos.buf));

	if (!haveLock)
		LockBuffer(so->currPos.buf, mbt_READ);

	page = BufferGetPage(so->currPos.buf);
	opaque = (mbtPageOpaque) PageGetSpecialPointer(page);
	minoff = P_FIRSTDATAKEY(opaque);
	maxoff = PageGetMaxOffsetNumber(page);

	for (i = 0; i < so->numKilled; i++)
	{
		int			itemIndex = so->killedItems[i];
		mbtScanPosItem *kitem = &so->currPos.items[itemIndex];
		OffsetNumber offnum = kitem->indexOffset;

		Assert(itemIndex >= so->currPos.firstItem &&
			   itemIndex <= so->currPos.lastItem);
		if (offnum < minoff)
			continue;			/* pure paranoia */
		while (offnum <= maxoff)
		{
			ItemId		iid = PageGetItemId(page, offnum);
			IndexTuple	ituple = (IndexTuple) PageGetItem(page, iid);

			if (ItemPointerEquals(&ituple->t_tid, &kitem->heapTid))
			{
				/* found the item */
				ItemIdMarkDead(iid);
				killedsomething = true;
				break;			/* out of inner search loop */
			}
			offnum = OffsetNumberNext(offnum);
		}
	}

	/*
	 * Since this can be redone later if needed, it's treated the same as a
	 * commit-hint-bit status update for heap tuples: we mark the buffer dirty
	 * but don't make a WAL log entry.
	 *
	 * Whenever we mark anything LP_DEAD, we also set the page's
	 * mbtP_HAS_GARBAGE flag, which is likewise just a hint.
	 */
	if (killedsomething)
	{
		opaque->mbtpo_flags |= mbtP_HAS_GARBAGE;
		SetBufferCommitInfoNeedsSave(so->currPos.buf);
	}

	if (!haveLock)
		LockBuffer(so->currPos.buf, BUFFER_LOCK_UNLOCK);

	/*
	 * Always reset the scan state, so we don't look for same items on other
	 * pages.
	 */
	so->numKilled = 0;
}


/*
 * The following routines manage a shared-memory area in which we track
 * assignment of "vacuum cycle IDs" to currently-active mbtree vacuuming
 * operations.	There is a single counter which increments each time we
 * start a vacuum to assign it a cycle ID.	Since multiple vacuums could
 * be active concurrently, we have to track the cycle ID for each active
 * vacuum; this requires at most MaxBackends entries (usually far fewer).
 * We assume at most one vacuum can be active for a given index.
 *
 * Access to the shared memory area is controlled by mbtreeVacuumLock.
 * In principle we could use a separate lmgr locktag for each index,
 * but a single LWLock is much cheaper, and given the short time that
 * the lock is ever held, the concurrency hit should be minimal.
 */

typedef struct mbtOneVacInfo
{
	LockRelId	relid;			/* global identifier of an index */
	mbtCycleId	cycleid;		/* cycle ID for its active VACUUM */
} mbtOneVacInfo;

typedef struct mbtVacInfo
{
	mbtCycleId	cycle_ctr;		/* cycle ID most recently assigned */
	int			num_vacuums;	/* number of currently active VACUUMs */
	int			max_vacuums;	/* allocated length of vacuums[] array */
	mbtOneVacInfo vacuums[1];	/* VARIABLE LENGTH ARRAY */
} mbtVacInfo;

static mbtVacInfo *mbtvacinfo;


/*
 * _mbt_vacuum_cycleid --- get the active vacuum cycle ID for an index,
 *		or zero if there is no active VACUUM
 *
 * Note: for correct interlocking, the caller must already hold pin and
 * exclusive lock on each buffer it will store the cycle ID into.  This
 * ensures that even if a VACUUM starts immediately afterwards, it cannot
 * process those pages until the page split is complete.
 */
mbtCycleId
_mbt_vacuum_cycleid(Relation rel)
{
	mbtCycleId	result = 0;
	int			i;

	/* Share lock is enough since this is a read-only operation */
	LWLockAcquire(mbtreeVacuumLock, LW_SHARED);

	for (i = 0; i < mbtvacinfo->num_vacuums; i++)
	{
		mbtOneVacInfo *vac = &mbtvacinfo->vacuums[i];

		if (vac->relid.relId == rel->rd_lockInfo.lockRelId.relId &&
			vac->relid.dbId == rel->rd_lockInfo.lockRelId.dbId)
		{
			result = vac->cycleid;
			break;
		}
	}

	LWLockRelease(mbtreeVacuumLock);
	return result;
}

/*
 * _mbt_start_vacuum --- assign a cycle ID to a just-starting VACUUM operation
 *
 * Note: the caller must guarantee that it will eventually call
 * _mbt_end_vacuum, else we'll permanently leak an array slot.  To ensure
 * that this happens even in elog(FATAL) scenarios, the appropriate coding
 * is not just a PG_TRY, but
 *		PG_ENSURE_ERROR_CLEANUP(_mbt_end_vacuum_callback, PointerGetDatum(rel))
 */
mbtCycleId
_mbt_start_vacuum(Relation rel)
{
	mbtCycleId	result;
	int			i;
	mbtOneVacInfo *vac;

	LWLockAcquire(mbtreeVacuumLock, LW_EXCLUSIVE);

	/*
	 * Assign the next cycle ID, being careful to avoid zero as well as the
	 * reserved high values.
	 */
	result = ++(mbtvacinfo->cycle_ctr);
	if (result == 0 || result > MAX_mbt_CYCLE_ID)
		result = mbtvacinfo->cycle_ctr = 1;

	/* Let's just make sure there's no entry already for this index */
	for (i = 0; i < mbtvacinfo->num_vacuums; i++)
	{
		vac = &mbtvacinfo->vacuums[i];
		if (vac->relid.relId == rel->rd_lockInfo.lockRelId.relId &&
			vac->relid.dbId == rel->rd_lockInfo.lockRelId.dbId)
		{
			/*
			 * Unlike most places in the backend, we have to explicitly
			 * release our LWLock before throwing an error.  This is because
			 * we expect _mbt_end_vacuum() to be called before transaction
			 * abort cleanup can run to release LWLocks.
			 */
			LWLockRelease(mbtreeVacuumLock);
			elog(ERROR, "multiple active vacuums for index \"%s\"",
				 RelationGetRelationName(rel));
		}
	}

	/* OK, add an entry */
	if (mbtvacinfo->num_vacuums >= mbtvacinfo->max_vacuums)
	{
		LWLockRelease(mbtreeVacuumLock);
		elog(ERROR, "out of mbtvacinfo slots");
	}
	vac = &mbtvacinfo->vacuums[mbtvacinfo->num_vacuums];
	vac->relid = rel->rd_lockInfo.lockRelId;
	vac->cycleid = result;
	mbtvacinfo->num_vacuums++;

	LWLockRelease(mbtreeVacuumLock);
	return result;
}

/*
 * _mbt_end_vacuum --- mark a mbtree VACUUM operation as done
 *
 * Note: this is deliberately coded not to complain if no entry is found;
 * this allows the caller to put PG_TRY around the start_vacuum operation.
 */
void
_mbt_end_vacuum(Relation rel)
{
	int			i;

	LWLockAcquire(mbtreeVacuumLock, LW_EXCLUSIVE);

	/* Find the array entry */
	for (i = 0; i < mbtvacinfo->num_vacuums; i++)
	{
		mbtOneVacInfo *vac = &mbtvacinfo->vacuums[i];

		if (vac->relid.relId == rel->rd_lockInfo.lockRelId.relId &&
			vac->relid.dbId == rel->rd_lockInfo.lockRelId.dbId)
		{
			/* Remove it by shifting down the last entry */
			*vac = mbtvacinfo->vacuums[mbtvacinfo->num_vacuums - 1];
			mbtvacinfo->num_vacuums--;
			break;
		}
	}

	LWLockRelease(mbtreeVacuumLock);
}

/*
 * _mbt_end_vacuum wrapped as an on_shmem_exit callback function
 */
void
_mbt_end_vacuum_callback(int code, Datum arg)
{
	_mbt_end_vacuum((Relation) DatumGetPointer(arg));
}

/*
 * mbtreeShmemSize --- report amount of shared memory space needed
 */
Size
mbtreeShmemSize(void)
{
	Size		size;

	size = offsetof(mbtVacInfo, vacuums[0]);
	size = add_size(size, mul_size(MaxBackends, sizeof(mbtOneVacInfo)));
	return size;
}

/*
 * mbtreeShmemInit --- initialize this module's shared memory
 */
void
mbtreeShmemInit(void)
{
	bool		found;

	mbtvacinfo = (mbtVacInfo *) ShmemInitStruct("mbtree Vacuum State",
											  mbtreeShmemSize(),
											  &found);

	if (!IsUnderPostmaster)
	{
		/* Initialize shared memory area */
		Assert(!found);

		/*
		 * It doesn't really matter what the cycle counter starts at, but
		 * having it always start the same doesn't seem good.  Seed with
		 * low-order bits of time() instead.
		 */
		mbtvacinfo->cycle_ctr = (mbtCycleId) time(NULL);

		mbtvacinfo->num_vacuums = 0;
		mbtvacinfo->max_vacuums = MaxBackends;
	}
	else
		Assert(found);
}

Datum
mbtoptions(PG_FUNCTION_ARGS)
{
	// Datum		reloptions = PG_GETARG_DATUM(0);
	// bool		validate = PG_GETARG_BOOL(1);
	// bytea	   *result;

	// result = default_reloptions(reloptions, validate, RELOPT_KIND_MBTREE);
	// if (result)
	// 	PG_RETURN_BYTEA_P(result);
	// PG_RETURN_NULL();
	printf("\nmbtoptions!!!\n");
	PG_RETURN_NULL();
}
