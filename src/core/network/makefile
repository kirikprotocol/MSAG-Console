
#
# $Id$
#
#

PACKAGE := core.network

CXXFLAGS := $(CXXFLAGS) -DDISABLE_ANY_CHECKS

LDFLAGS=$(LDGLAGS) -lsocket

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
