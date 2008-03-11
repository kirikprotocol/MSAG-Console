#
# $Id$
#
#-DURCREGISTRY_GLOBAL_INIT
CXXFLAGS := -I$(SS7_STACK_HOME)/src $(CXXFLAGS)
CFLAGS := -I$(SS7_STACK_HOME)/src $(CFLAGS)
PACKAGE := inman.inap

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
