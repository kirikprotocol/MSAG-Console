#
# $Id$
#
#CXXFLAGS := $(CXXFLAGS)
PACKAGE := inman.abprov.db

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions -ldb_exc -L$(SMSC_BUILDDIR)/bin/inman/ldlib -linman_iap_fclt -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
