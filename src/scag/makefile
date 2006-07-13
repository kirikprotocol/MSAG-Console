
#
# $Id$
#
#

PACKAGE := scag

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/exc/ -lscag_exc

CXXFLAGS := -DSMPPGW $(CXXFLAGS)

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

ifeq ($(TESTING),YES)
CXXFLAGS := $(CXXFLAGS) -DTESTING
endif

include $(SMSC_SRCDIR)/conf/module_rules
