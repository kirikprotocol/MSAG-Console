
#
# $Id$
#
#

#CXXFLAGS=-I$(SMSC_SRCDIR)

PACKAGE := db.bdb_framework

ifndef BDB_SRCDIR
BDB_SRCDIR=/home/shulga/project/berkeleyDB
endif

CXXFLAGS := $(CXXFLAGS) -I$(BDB_SRCDIR)/include

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
