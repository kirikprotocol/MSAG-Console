
#
# $Id$
#
#

PACKAGE := core.test

CXXFLAGS := $(CXXFLAGS) -DUSEPOOL -D_REENTERANT -DDISABLE_ANY_CHECKS

__LIBRARIES :=-L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
