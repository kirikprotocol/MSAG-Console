
#
# $Id$
#
#

PACKAGE := scag.re.testing


#__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc
#__OCI_LIB_SO__ :=  $(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc.so
__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/exc/ -libre_exc

CXXFLAGS:= $(CXXFLAGS) -I/export/home/green/js/src/js


ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
