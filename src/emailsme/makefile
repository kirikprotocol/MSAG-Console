
#
# $Id$
#
#

PACKAGE := emailsme

__LIBRARIES :=-L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CXXFLAGS := $(CXXFLAGS) -DNAMEDBRACKETS
include $(SMSC_SRCDIR)/conf/module_rules
