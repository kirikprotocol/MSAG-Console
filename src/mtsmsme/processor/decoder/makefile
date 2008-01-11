CFLAGS := -I../ros -I../tcap -I../codec -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_TCMessage
CXXFLAGS := -I../ros -I../tcap -I../codec -I../asn1rt $(CFLAGS) -Dasn_DEF=asn_DEF_TCMessage
PACKAGE := mtsmsme.processor.decoder

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
