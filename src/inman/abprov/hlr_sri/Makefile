#
# $Id$
#

#CXXFLAGS := $(CXXFLAGS)

PACKAGE := inman.abprov.hlr_sri

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/inman/abprov -linman_iap_fclt -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
