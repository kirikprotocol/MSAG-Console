
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR) -I$(SMSC_SRCDIR)/sms -I$(SMSC_SRCDIR)/scag/util/cluster -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public
PACKAGE := scag.stat

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules