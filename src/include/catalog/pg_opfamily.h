/*-------------------------------------------------------------------------
 *
 * pg_opfamily.h
 *	  definition of the system "opfamily" relation (pg_opfamily)
 *	  along with the relation's initial contents.
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/pg_opfamily.h,v 1.11 2009/01/01 17:23:57 momjian Exp $
 *
 * NOTES
 *	  the genbki.sh script reads this file and generates .bki
 *	  information from the DATA() statements.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_OPFAMILY_H
#define PG_OPFAMILY_H

#include "catalog/genbki.h"

/* ----------------
 *		pg_opfamily definition. cpp turns this into
 *		typedef struct FormData_pg_opfamily
 * ----------------
 */
#define OperatorFamilyRelationId  2753

CATALOG(pg_opfamily,2753)
{
	Oid			opfmethod;		/* index access method opfamily is for */
	NameData	opfname;		/* name of this opfamily */
	Oid			opfnamespace;	/* namespace of this opfamily */
	Oid			opfowner;		/* opfamily owner */
} FormData_pg_opfamily;

/* ----------------
 *		Form_pg_opfamily corresponds to a pointer to a tuple with
 *		the format of pg_opfamily relation.
 * ----------------
 */
typedef FormData_pg_opfamily *Form_pg_opfamily;

/* ----------------
 *		compiler constants for pg_opfamily
 * ----------------
 */
#define Natts_pg_opfamily				4
#define Anum_pg_opfamily_opfmethod		1
#define Anum_pg_opfamily_opfname		2
#define Anum_pg_opfamily_opfnamespace	3
#define Anum_pg_opfamily_opfowner		4

/* ----------------
 *		initial contents of pg_opfamily
 * ----------------
 */

DATA(insert OID =  421 (	403		abstime_ops		PGNSP PGUID ));
DATA(insert OID =  397 (	403		array_ops		PGNSP PGUID ));
DATA(insert OID =  423 (	403		bit_ops			PGNSP PGUID ));
DATA(insert OID =  424 (	403		bool_ops		PGNSP PGUID ));
#define BOOL_BTREE_FAM_OID 424
DATA(insert OID =  426 (	403		bpchar_ops		PGNSP PGUID ));
#define BPCHAR_BTREE_FAM_OID 426
DATA(insert OID =  427 (	405		bpchar_ops		PGNSP PGUID ));
DATA(insert OID =  428 (	403		bytea_ops		PGNSP PGUID ));
#define BYTEA_BTREE_FAM_OID 428
DATA(insert OID =  429 (	403		char_ops		PGNSP PGUID ));
DATA(insert OID =  431 (	405		char_ops		PGNSP PGUID ));
DATA(insert OID =  434 (	403		datetime_ops	PGNSP PGUID ));
DATA(insert OID =  435 (	405		date_ops		PGNSP PGUID ));
DATA(insert OID = 1970 (	403		float_ops		PGNSP PGUID ));
DATA(insert OID = 1971 (	405		float_ops		PGNSP PGUID ));
DATA(insert OID = 1974 (	403		network_ops		PGNSP PGUID ));
#define NETWORK_BTREE_FAM_OID 1974
DATA(insert OID = 1975 (	405		network_ops		PGNSP PGUID ));
DATA(insert OID = 1976 (	403		integer_ops		PGNSP PGUID ));
#define INTEGER_BTREE_FAM_OID 1976
DATA(insert OID = 1977 (	405		integer_ops		PGNSP PGUID ));
DATA(insert OID = 1982 (	403		interval_ops	PGNSP PGUID ));
DATA(insert OID = 1983 (	405		interval_ops	PGNSP PGUID ));
DATA(insert OID = 1984 (	403		macaddr_ops		PGNSP PGUID ));
DATA(insert OID = 1985 (	405		macaddr_ops		PGNSP PGUID ));
DATA(insert OID = 1986 (	403		name_ops		PGNSP PGUID ));
#define NAME_BTREE_FAM_OID 1986
DATA(insert OID = 1987 (	405		name_ops		PGNSP PGUID ));
DATA(insert OID = 1988 (	403		numeric_ops		PGNSP PGUID ));
DATA(insert OID = 1998 (	405		numeric_ops		PGNSP PGUID ));
DATA(insert OID = 1989 (	403		oid_ops			PGNSP PGUID ));
#define OID_BTREE_FAM_OID 1989
DATA(insert OID = 1990 (	405		oid_ops			PGNSP PGUID ));
DATA(insert OID = 1991 (	403		oidvector_ops	PGNSP PGUID ));
DATA(insert OID = 1992 (	405		oidvector_ops	PGNSP PGUID ));
DATA(insert OID = 2994 (	403		record_ops		PGNSP PGUID ));
DATA(insert OID = 1994 (	403		text_ops		PGNSP PGUID ));
#define TEXT_BTREE_FAM_OID 1994
DATA(insert OID = 1995 (	405		text_ops		PGNSP PGUID ));
DATA(insert OID = 1996 (	403		time_ops		PGNSP PGUID ));
DATA(insert OID = 1997 (	405		time_ops		PGNSP PGUID ));
DATA(insert OID = 1999 (	405		timestambtz_ops PGNSP PGUID ));
DATA(insert OID = 2000 (	403		timetz_ops		PGNSP PGUID ));
DATA(insert OID = 2001 (	405		timetz_ops		PGNSP PGUID ));
DATA(insert OID = 2002 (	403		varbit_ops		PGNSP PGUID ));
DATA(insert OID = 2040 (	405		timestamp_ops	PGNSP PGUID ));
DATA(insert OID = 2095 (	403		text_pattern_ops	PGNSP PGUID ));
#define TEXT_PATTERN_BTREE_FAM_OID 2095
DATA(insert OID = 2097 (	403		bpchar_pattern_ops	PGNSP PGUID ));
#define BPCHAR_PATTERN_BTREE_FAM_OID 2097
DATA(insert OID = 2099 (	403		money_ops		PGNSP PGUID ));
DATA(insert OID = 2222 (	405		bool_ops		PGNSP PGUID ));
#define BOOL_HASH_FAM_OID 2222
DATA(insert OID = 2223 (	405		bytea_ops		PGNSP PGUID ));
DATA(insert OID = 2224 (	405		int2vector_ops	PGNSP PGUID ));
DATA(insert OID = 2789 (	403		tid_ops			PGNSP PGUID ));
DATA(insert OID = 2225 (	405		xid_ops			PGNSP PGUID ));
DATA(insert OID = 2226 (	405		cid_ops			PGNSP PGUID ));
DATA(insert OID = 2227 (	405		abstime_ops		PGNSP PGUID ));
DATA(insert OID = 2228 (	405		reltime_ops		PGNSP PGUID ));
DATA(insert OID = 2229 (	405		text_pattern_ops	PGNSP PGUID ));
DATA(insert OID = 2231 (	405		bpchar_pattern_ops	PGNSP PGUID ));
DATA(insert OID = 2233 (	403		reltime_ops		PGNSP PGUID ));
DATA(insert OID = 2234 (	403		tinterval_ops	PGNSP PGUID ));
DATA(insert OID = 2235 (	405		aclitem_ops		PGNSP PGUID ));
DATA(insert OID = 2593 (	783		box_ops			PGNSP PGUID ));
DATA(insert OID = 2594 (	783		poly_ops		PGNSP PGUID ));
DATA(insert OID = 2595 (	783		circle_ops		PGNSP PGUID ));
DATA(insert OID = 2745 (	2742	array_ops		PGNSP PGUID ));
DATA(insert OID = 2968 (	403		uuid_ops		PGNSP PGUID ));
DATA(insert OID = 2969 (	405		uuid_ops		PGNSP PGUID ));
DATA(insert OID = 3522 (	403		enum_ops		PGNSP PGUID ));
DATA(insert OID = 3523 (	405		enum_ops		PGNSP PGUID ));
DATA(insert OID = 3626 (	403		tsvector_ops	PGNSP PGUID ));
DATA(insert OID = 3655 (	783		tsvector_ops	PGNSP PGUID ));
DATA(insert OID = 3659 (	2742	tsvector_ops	PGNSP PGUID ));
DATA(insert OID = 3683 (	403		tsquery_ops		PGNSP PGUID ));
DATA(insert OID = 3702 (	783		tsquery_ops		PGNSP PGUID ));

//??????
DATA(insert OID =  422 (	3303		abstime_ops		PGNSP PGUID ));
DATA(insert OID =  398 (	3303		array_ops		PGNSP PGUID ));
DATA(insert OID =  419 (	3303		bit_ops			PGNSP PGUID ));
DATA(insert OID =  420 (	3303		bool_ops		PGNSP PGUID ));
DATA(insert OID =  425 (	3303		bpchar_ops		PGNSP PGUID ));
DATA(insert OID =  430 (	3303		bytea_ops		PGNSP PGUID ));
DATA(insert OID =  432 (	3303		char_ops		PGNSP PGUID ));
DATA(insert OID =  436 (	3303		datetime_ops	PGNSP PGUID ));
DATA(insert OID = 1972 (	3303		float_ops		PGNSP PGUID ));
DATA(insert OID = 1973 (	3303		network_ops		PGNSP PGUID ));
DATA(insert OID = 1978 (	3303		integer_ops		PGNSP PGUID ));
DATA(insert OID = 1981 (	3303		interval_ops	PGNSP PGUID ));
DATA(insert OID = 2009 (	3303		macaddr_ops		PGNSP PGUID ));
DATA(insert OID = 1993 (	3303		name_ops		PGNSP PGUID ));
DATA(insert OID = 2003 (	3303		numeric_ops		PGNSP PGUID ));
// DATA(insert OID = 2008 (	3303		oid_ops			PGNSP PGUID ));
// DATA(insert OID = 1998 (	3303		oidvector_ops	PGNSP PGUID ));
DATA(insert OID = 2995 (	3303		record_ops		PGNSP PGUID ));
DATA(insert OID = 2004 (	3303		text_ops		PGNSP PGUID ));
DATA(insert OID = 2005 (	3303		time_ops		PGNSP PGUID ));
DATA(insert OID = 2007 (	3303		timetz_ops		PGNSP PGUID ));
DATA(insert OID = 2006 (	3303		varbit_ops		PGNSP PGUID ));
DATA(insert OID = 2242 (	3303		text_pattern_ops	PGNSP PGUID ));
DATA(insert OID = 2243 (	3303		bpchar_pattern_ops	PGNSP PGUID ));
DATA(insert OID = 2100 (	3303		money_ops		PGNSP PGUID ));
DATA(insert OID = 2790 (	3303		tid_ops			PGNSP PGUID ));
DATA(insert OID = 2241 (	3303		reltime_ops		PGNSP PGUID ));
DATA(insert OID = 2236 (	3303		tinterval_ops	PGNSP PGUID ));
DATA(insert OID = 2970 (	3303		uuid_ops		PGNSP PGUID ));
DATA(insert OID = 3524 (	3303		enum_ops		PGNSP PGUID ));
DATA(insert OID = 3627 (	3303		tsvector_ops	PGNSP PGUID ));
DATA(insert OID = 3684 (	3303		tsquery_ops		PGNSP PGUID ));

// DATA(insert OID =  425 (	3303	bpchar_ops		PGNSP PGUID ));
// DATA(insert OID =  432 (	3303	char_ops		PGNSP PGUID ));
// DATA(insert OID =  436 (	3303		date_ops		PGNSP PGUID ));
// DATA(insert OID = 1972 (	3303		float_ops		PGNSP PGUID ));
// DATA(insert OID = 1973 (	3303		network_ops		PGNSP PGUID ));
// DATA(insert OID = 1978 (	3303		integer_ops		PGNSP PGUID ));
// DATA(insert OID = 1981 (	3303		interval_ops	PGNSP PGUID ));
// DATA(insert OID = 2009 (	3303		macaddr_ops		PGNSP PGUID ));
// DATA(insert OID = 1993 (	3303		name_ops		PGNSP PGUID ));
// DATA(insert OID = 2003 (	3303		numeric_ops		PGNSP PGUID ));
// //DATA(insert OID = 2008 (	3303		oid_ops			PGNSP PGUID ));
// //DATA(insert OID = 1998 (	3303		oidvector_ops	PGNSP PGUID ));
// DATA(insert OID = 2004 (	3303		text_ops		PGNSP PGUID ));
// DATA(insert OID = 2005 (	3303		time_ops		PGNSP PGUID ));
// DATA(insert OID = 2006 (	3303		timestambtz_ops PGNSP PGUID ));
// DATA(insert OID = 2007 (	3303		timetz_ops		PGNSP PGUID ));
// DATA(insert OID = 2041 (	3303		timestamp_ops	PGNSP PGUID ));
// DATA(insert OID = 2230 (	3303		bool_ops		PGNSP PGUID ));
// DATA(insert OID = 2236 (	3303		bytea_ops		PGNSP PGUID ));
// DATA(insert OID = 2237 (	3303		int2vector_ops	PGNSP PGUID ));
// DATA(insert OID = 2238 (	3303		xid_ops			PGNSP PGUID ));
// DATA(insert OID = 2239 (	3303		cid_ops			PGNSP PGUID ));
// DATA(insert OID = 2240 (	3303		abstime_ops		PGNSP PGUID ));
// DATA(insert OID = 2241 (	3303		reltime_ops		PGNSP PGUID ));
// DATA(insert OID = 2242 (	3303		text_pattern_ops	PGNSP PGUID ));
// DATA(insert OID = 2243 (	3303		bpchar_pattern_ops	PGNSP PGUID ));
// DATA(insert OID = 2244 (	3303		aclitem_ops		PGNSP PGUID ));
// DATA(insert OID = 2970 (	3303		uuid_ops		PGNSP PGUID ));
// DATA(insert OID = 3524 (	3303		enum_ops		PGNSP PGUID ));

#endif   /* PG_OPFAMILY_H */
