CFLAGS := -I../codec -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_MessageType
CXXFLAGS := -I../codec -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_MessageType
PACKAGE := mtsmsme.processor.decoder

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
