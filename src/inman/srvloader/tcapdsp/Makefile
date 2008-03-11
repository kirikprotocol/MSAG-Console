#
# $Id$
#

#CXXFLAGS := $(CXXFLAGS)

PACKAGE := inman.srvloader.tcapdsp
#shared libs used
__LIBRARIES := -L$(SS7_STACK_HOME)/lib -l$(SS7_LIBRARY_NAME) -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
