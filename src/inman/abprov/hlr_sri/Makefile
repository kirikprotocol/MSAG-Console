#
# $Id$
#
#CXXFLAGS := $(CXXFLAGS)
PACKAGE := inman.abprov.hlr_sri

__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/inman/ldlib -linman_iap_fclt -linman_ics_tcapdsp -L$(SMSC_BUILDDIR)/bin -llogger

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
