
#
# $Id$
#
#
#SS7_STACK_HOME := /opt/EABss7009
#export SS7_STACK_HOME
#SS7_LIBRARY_NAME := einss7
#export SS7_LIBRARY_NAME

CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR)/sms -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public -I/opt/EINss7166/src
PACKAGE := mcisme

__LIBRARIES       := -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc -L$(SMSC_BUILDDIR)/bin -L$(SS7_STACK_HOME)/lib
__OCI_LIB_SO__    :=  $(ORACLE_LIB)/libclntsh.so

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	

