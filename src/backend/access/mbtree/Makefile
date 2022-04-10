#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for access/mbt
#
# IDENTIFICATION
#    $PostgreSQL: pgsql/src/backend/access/mbt/Makefile,v 1.15 2008/03/16 23:15:08 tgl Exp $
#
#-------------------------------------------------------------------------

subdir = src/backend/access/mbtree
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

OBJS = mbtree.o  mbtreeinsert.o  mbtreepage.o mbtreexlog.o \
       mbtreesort.o mbtreeutils.o mbtreecompare.o mbtreesearch.o

include $(top_srcdir)/src/backend/common.mk
