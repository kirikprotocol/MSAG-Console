
#
# $Id$
#
#

PACKAGE := system

CXXFLAGS := $(CXXFLAGS) -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public

__LIBRARIES := $(ORACLE_LIB)/libclntsh.so -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc

ifneq ("$(NOMAP)","YES")
ifeq ("$(MACHTYPE)","sparc-sun-solaris")
__LIBRARIES := $(__LIBRARIES) -L$(SS7_STACK_HOME)/lib -leinss7_r -lsystem-mapio
CXXFLAGS := $(CXXFLAGS) -DUSE_MAPIO
endif
endif

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
