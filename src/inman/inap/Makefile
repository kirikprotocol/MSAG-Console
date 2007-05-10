#
# $Id$
#

CXXFLAGS := -I$(SS7_STACK_HOME)/src $(CXXFLAGS) -DURCREGISTRY_GLOBAL_INIT
CFLAGS := -I$(SS7_STACK_HOME)/src $(CFLAGS) -DURCREGISTRY_GLOBAL_INIT
PACKAGE := inman.inap

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
