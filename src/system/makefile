
#
# $Id$
#
#

PACKAGE := system

CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public -Imapio/inc

__LIBRARIES := $(ORACLE_LIB)/libclntsh.so $(SMSC_BUILDDIR)/bin/db/exceptions/libdb_exc.so 
ifeq ("$(MACHTYPE)","sparc-sun-solaris") 
__LIBRARIES := $(__LIBRARIES) -L/opt/EINss7166/lib -leinss7 -lsystem-mapio
CXXFLAGS := $(CXXFLAGS) -DUSE_MAPIO
endif
ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
