
#
# $Id$
#
#

PACKAGE := mmbox

ifndef BDB_SRCDIR
BDB_SRCDIR=/home/shulga/project/berkeleyDB
endif

CXXFLAGS := $(CXXFLAGS) -I$(BDB_SRCDIR)/include

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin -L$(BDB_SRCDIR)/lib 

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
