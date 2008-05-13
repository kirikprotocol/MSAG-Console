
CFLAGS := -I. -I$(SMSC_SRCDIR)/mtsmsme/processor/asn1rt -I$(SMSC_SRCDIR)/mtsmsme/codec/mapbscode -I$(SMSC_SRCDIR)/mtsmsme/codec/maptscode -I$(SMSC_SRCDIR)/mtsmsme/codec/mapextensiondatatypes $(CFLAGS)
CXXFLAGS := -I. -I$(SMSC_SRCDIR)/mtsmsme/processor/asn1rt -I$(SMSC_SRCDIR)/mtsmsme/codec/mapbscode -I$(SMSC_SRCDIR)/mtsmsme/codec/maptscode -I$(SMSC_SRCDIR)/mtsmsme/codec/mapextensiondatatypes $(CXXFLAGS)
PACKAGE := mtsmsme.codec.mapcommondatatypes

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
