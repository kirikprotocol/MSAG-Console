
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I/usr/local/include/xercesc
PACKAGE := admin.smsc_service

#__LIBRARIES := $(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules

