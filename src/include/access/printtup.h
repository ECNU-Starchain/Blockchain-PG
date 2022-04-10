/*-------------------------------------------------------------------------
 *
 * printtup.h
 *
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/printtup.h,v 1.38 2009/01/01 17:23:56 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PRINTTUP_H
#define PRINTTUP_H

#include "utils/portal.h"

/* ----------------------------------------------------------------
 *		printtup / debugtup support
 * ----------------------------------------------------------------
 */

/* ----------------
 *		Private state for a printtup destination object
 *
 * NOTE: finfo is the lookup info for either typoutput or typsend, whichever
 * we are using for this column.
 * ----------------
 */
typedef struct
{								/* Per-attribute information */
    Oid			typoutput;		/* Oid for the type's text output fn */
    Oid			typsend;		/* Oid for the type's binary output fn */
    bool		typisvarlena;	/* is it varlena (ie possibly toastable)? */
    int16		format;			/* format code for this column */
    FmgrInfo	finfo;			/* Precomputed call info for output fn */
} PrinttupAttrInfo;

typedef struct
{
    DestReceiver pub;			/* publicly-known function pointers */
    Portal		portal;			/* the Portal we are printing from */
    bool		sendDescrip;	/* send RowDescription at startup? */
    TupleDesc	attrinfo;		/* The attr info we are set up for */
    int			nattrs;
    PrinttupAttrInfo *myinfo;	/* Cached info about each attr */
} DR_printtup;

extern DestReceiver *printtup_create_DR(CommandDest dest);

extern void SetRemoteDestReceiverParams(DestReceiver *self, Portal portal);

extern void SendRowDescriptionMessage(TupleDesc typeinfo, List *targetlist,
						  int16 *formats);

extern void debugStartup(DestReceiver *self, int operation,
			 TupleDesc typeinfo);
extern void debugtup(TupleTableSlot *slot, DestReceiver *self);

/* XXX these are really in executor/spi.c */
extern void spi_dest_startup(DestReceiver *self, int operation,
				 TupleDesc typeinfo);
extern void spi_printtup(TupleTableSlot *slot, DestReceiver *self);
//by zcy modified 2020/6/30
#define DEFAULTSIZE 8192
//modified end	

/*
 * Get the lookup info that printtup() needs
 */
static void
printtup_prepare_info(DR_printtup *myState, TupleDesc typeinfo, int numAttrs)
{
    int16	   *formats = myState->portal->formats;
    int			i;

    /* get rid of any old data */
    if (myState->myinfo)
        pfree(myState->myinfo);
    myState->myinfo = NULL;

    myState->attrinfo = typeinfo;
    myState->nattrs = numAttrs;
    if (numAttrs <= 0)
        return;

    myState->myinfo = (PrinttupAttrInfo *)
        palloc0(numAttrs * sizeof(PrinttupAttrInfo));

    for (i = 0; i < numAttrs; i++)
    {
        PrinttupAttrInfo *thisState = myState->myinfo + i;
        int16		format = (formats ? formats[i] : 0);

        thisState->format = format;
        if (format == 0)
        {
            getTypeOutputInfo(typeinfo->attrs[i]->atttypid,
                              &thisState->typoutput,
                              &thisState->typisvarlena);
            fmgr_info(thisState->typoutput, &thisState->finfo);
        }
        else if (format == 1)
        {
            getTypeBinaryOutputInfo(typeinfo->attrs[i]->atttypid,
                                    &thisState->typsend,
                                    &thisState->typisvarlena);
            fmgr_info(thisState->typsend, &thisState->finfo);
        }
        else
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("unsupported format code: %d", format)));
    }
}

#endif   /* PRINTTUP_H */
