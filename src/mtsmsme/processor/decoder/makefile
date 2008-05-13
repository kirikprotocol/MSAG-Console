CFLAGS := -I$(SMSC_SRCDIR)/mtsmsme/codec/ros -I$(SMSC_SRCDIR)/mtsmsme/codec/tcap -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_TCMessage
CXXFLAGS := -I$(SMSC_SRCDIR)/mtsmsme/codec/ros -I$(SMSC_SRCDIR)/mtsmsme/codec/tcap -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_TCMessage
PACKAGE := mtsmsme.processor.decoder

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
