#
# $Id$
#

PACKAGE := inman

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin -L$(SS7_STACK_HOME)/lib -L$(SMSC_BUILDDIR)/bin/db/exceptions/ -ldb_exc
__OCI_LIB_SO__ :=  $(ORACLE_LIB)32/libclntsh.so

CXXFLAGS:=$(CXXFLAGS) -I./asn1rt -Icodec -D__EXTENSIONS__ -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public


ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
