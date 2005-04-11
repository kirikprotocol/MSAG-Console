CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR)/sms -I/opt/EABss7009/src -I./codec -I./decoder -Dasn_DEF=asn_DEF_MessageType
__LIBRARIES := $(__LIBRARIES) -L/opt/EABss7009/lib -L$(SMSC_BUILDDIR)/bin
PACKAGE := mtsmsme.processor

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
