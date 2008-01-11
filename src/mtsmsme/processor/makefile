# $Id$
CXXFLAGS := $(CXXFLAGS) -D__EXTENSIONS__ -I$(SMSC_SRCDIR)/sms -I/opt/EABss7009/src -I./ros -I./tcap -I./codec -I./asn1rt -I./decoder -I./ulcodec -Dasn_DEF=asn_DEF_TCMessage
__LIBRARIES := $(__LIBRARIES) -L$(SS7_STACK_HOME)/lib -L$(SMSC_BUILDDIR)/bin
PACKAGE := mtsmsme.processor

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
