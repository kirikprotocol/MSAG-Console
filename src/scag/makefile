
#
# $Id$
#
#

PACKAGE := scag

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc

CXXFLAGS := -DSMPPGW $(CXXFLAGS)

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
