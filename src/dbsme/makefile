
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I/usr/local/include/xercesc -I$(SMSC_SRCDIR)/sms -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public
PACKAGE := dbsme

__LIBRARIES := $(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so
__OCI_LIB_SO__ :=  $(ORACLE_HOME)/lib/libclntsh.so

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	

