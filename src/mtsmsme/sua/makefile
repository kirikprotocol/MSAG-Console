# $Id$
CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR) -I$(SMSC_SRCDIR)/mtsmsme/processor/asn1rt -I$(SMSC_SRCDIR)/mtsmsme/codec/tcap -I$(SMSC_SRCDIR)/mtsmsme/codec/ros
__LIBRARIES := $(__LIBRARIES) -L$(SS7_STACK_HOME)/lib
PACKAGE := mtsmsme.sua

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules


	

