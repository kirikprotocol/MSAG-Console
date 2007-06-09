
#
# $Id$
#
#

#CXXFLAGS=-I$(SMSC_SRCDIR)

PACKAGE := mmbox.app_packets_handler

ifndef BDB_SRCDIR
BDB_SRCDIR=/home/shulga/project/berkeleyDB
endif

CXXFLAGS := $(CXXFLAGS) -I$(BDB_SRCDIR)/include

#__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/db/exceptions -ldb_exc -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
