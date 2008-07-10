
#
# $Id$
#
#

PACKAGE := core.network

CXXFLAGS := $(CXXFLAGS) -DDISABLE_ANY_CHECKS

ifneq ($(shell uname),Linux)
LDFLAGS := $(LDFLAGS) -lsocket
endif

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
