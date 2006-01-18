
#
# $Id$
#
#

PACKAGE := scag.gt

JS_SRCDIR:=$(SMSC_SRCDIR)/scag/gt/js

#__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc
#__OCI_LIB_SO__ :=  $(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc.so
__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/exc/ -libre_exc -L$(SMSC_BUILDDIR)/bin -ljslib.so
CXXFLAGS:= $(CXXFLAGS) -I$(JS_SRCDIR) 

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
