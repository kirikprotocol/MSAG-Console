#
# $Id$
#

PACKAGE := inman.codecs

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin 

CXXFLAGS:= -I../codec_inc/ros -I../codec_inc/in_cs -I../codec_inc/map -I../asn1rt -DASN1_XER_NOT_USED $(CXXFLAGS)


ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
