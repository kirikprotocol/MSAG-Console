#
# $Id$
#

PACKAGE := inman.codec_inc

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin 

CXXFLAGS:= -I./ros -I./in_cs -I./map -I../asn1rt -DASN1_XER_NOT_USED $(CXXFLAGS)


ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
