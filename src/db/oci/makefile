
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I. -I$(SMSC_SRCDIR)/dbsme -I/usr/local/include/xercesc -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public
PACKAGE := db.oci

__LIBRARIES := -L$(ORACLE_LIB) -lclntsh -L$(SMSC_BUILDDIR)/bin/db/exceptions -ldb_exc -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
