#
# $Id$
#
CXXFLAGS := -I$(SS7_STACK_HOME)/src  $(CXXFLAGS)
CFLAGS := -I$(SS7_STACK_HOME)/src $(CFLAGS)
PACKAGE := inman.inap.cap_sms

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
