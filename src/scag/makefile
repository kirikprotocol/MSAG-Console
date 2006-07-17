
#
# $Id$
#
#

PACKAGE := scag

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

ifeq ($(TESTING),YES)
CXXFLAGS := $(CXXFLAGS) -DTESTING
endif

include $(SMSC_SRCDIR)/conf/module_rules
