#
# $Id$
#

#CXXFLAGS := $(CXXFLAGS)

PACKAGE := inman.srvloader.smbill
#shared libs used
__LIBRARIES := -L$(SMSC_BUILDDIR)/bin -llogger -L$(SMSC_BUILDDIR)/bin/inman/ldlib -linman_ics_tcapdsp

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
