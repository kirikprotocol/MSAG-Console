
#
# $Id$
#
#

ifeq ($(CXX),CC)
CXXFLAGS := -g -xtarget=ultra3 -xarch=v9 -xmemalign=8s -xcode=pic32 $(CXXFLAGS) -I.
else
# gcc
CXXFLAGS := -g -fPIC $(CXXFLAGS) -I.
endif

PACKAGE := db.fileStorage

#__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions -ldb_exc -L$(SMSC_BUILDDIR)/bin -llogger
#XTARGET="-g -xtarget=ultra3 -xarch=v9 -xmemalign=8s -xcode=pic32"

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
