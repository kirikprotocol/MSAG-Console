
CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR)/sms 
__LIBRARIES := $(__LIBRARIES) -L/opt/EABss7009/lib -L$(SMSC_BUILDDIR)/bin
PACKAGE := mtsmsme

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	

