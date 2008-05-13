# $Id$
CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR) -I$(SMSC_SRCDIR)/mtsmsme/codec/ros -I$(SMSC_SRCDIR)/mtsmsme/codec/tcap  -I../processor/asn1rt -I../processor -I$(SMSC_SRCDIR)/mtsmsme/codec/mapbscode -I$(SMSC_SRCDIR)/mtsmsme/codec/maptscode -I$(SMSC_SRCDIR)/mtsmsme/codec/mapextensiondatatypes -I$(SMSC_SRCDIR)/mtsmsme/codec/mapcommondatatypes -I$(SMSC_SRCDIR)/mtsmsme/codec/maperdatatypes -I$(SMSC_SRCDIR)/mtsmsme/codec/mapsmdatatypes -I$(SMSC_SRCDIR)/mtsmsme/codec/mapmsdatatypes
__LIBRARIES := $(__LIBRARIES) -L$(SS7_STACK_HOME)/lib
PACKAGE := mtsmsme.comp

LDFLAGS := $(LDFLAGS) -z verbose

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	

