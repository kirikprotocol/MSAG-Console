#
# $Id$
#

CFLAGS := -I. -I../ros -I../in_cs -I../map -I../../asn1rt -DASN1_XER_NOT_USED $(CFLAGS)
CXXFLAGS := -I. -I../ros -I../in_cs -I../map -I../../asn1rt -DASN1_XER_NOT_USED $(CXXFLAGS)
PACKAGE := inman.codec_inc.cap

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
