
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR)/sms -I$(SMSC_SRCDIR)/store -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public

PACKAGE := tests

__LIBS := -L$(ORACLE_LIB) $(XERCES_LIB) $(ORACLE_LIB)/libclntsh.so -lthread -lrt $(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	


