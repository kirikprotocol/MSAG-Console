
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -g
PACKAGE := logger

ifeq ($(LOGGER_TIMED_CONFIG_RELOAD),YES)
CXXFLAGS := $(CXXFLAGS) -DLOGGER_TIMED_CONFIG_RELOAD -I.
endif

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
