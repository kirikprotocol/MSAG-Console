#
# $Id$
#

PACKAGE := inman

__LIBRARIES       := -L$(SMSC_BUILDDIR)/bin -L$(SS7_STACK_HOME)/lib

CXXFLAGS:=$(CXXFLAGS) -I./asn1rt -Icodec

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules
