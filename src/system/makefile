
#
# $Id$
#
#

PACKAGE := system

CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public -Imapio/inc

__LIBRARIES := $(ORACLE_LIB)/libclntsh.so $(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
