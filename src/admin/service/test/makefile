
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I/usr/local/include/xercesc
PACKAGE := admin.service.test

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

#BINARIES := $(SMSC_BUILDDIR)/bin/admin/service/libsmsc_dumb_service.so

#$(SMSC_BUILDDIR)/bin/admin/service/libsmsc_dumb_service.so: $(SMSC_BUILDDIR)/obj/libadmin-service-test/ServiceSocketListenerTest.o
#	$(CXX) -shared $(LDFLAGS) -o $@ $(SMSC_BUILDDIR)/obj/libadmin-service-test/ServiceSocketListenerTest.o

include $(SMSC_SRCDIR)/conf/module_rules

#$(warning $(BINARIES))




