
#
# $Id$
#
#

PACKAGE := system.abonentinfo

CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public -Imapio/inc

__LIBRARIES := $(ORACLE_LIB)/libclntsh.so -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc 

ifne ("$(NOMAP)","YES")
ifeq ("$(MACHTYPE)","sparc-sun-solaris") 
__LIBRARIES := $(__LIBRARIES) -L/opt/EINss7166/lib -leinss7
CXXFLAGS := $(CXXFLAGS) -DUSE_MAPIO
endif
endif

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif


include $(SMSC_SRCDIR)/conf/module_rules
