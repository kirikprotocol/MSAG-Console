
CFLAGS := -I. -I$(SMSC_SRCDIR)/mtsmsme/processor/asn1rt -I$(SMSC_SRCDIR)/mtsmsme/codec/ros $(CFLAGS)
CXXFLAGS := -I. -I$(SMSC_SRCDIR)/mtsmsme/processor/asn1rt -I$(SMSC_SRCDIR)/mtsmsme/codec/ros $(CXXFLAGS)
PACKAGE := mtsmsme.codec.tcap

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
