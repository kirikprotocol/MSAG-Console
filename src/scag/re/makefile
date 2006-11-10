
#
# $Id$
#
#
CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR) -I$(SMSC_SRCDIR)/sms -I$(SMSC_SRCDIR)/core/buffers -I$(SMSC_SRCDIR)/scag/util/cluster


PACKAGE := scag.re


#__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc
#__OCI_LIB_SO__ :=  $(SMSC_BUILDDIR)/bin/scag/re/exc/ -libre_exc.so
__LIBRARIES := -L$(SMSC_BUILDDIR)/bin/scag/exc/ -libre_exc


ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
