
#
# $Id$
#
#

PACKAGE := system.mapio

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CXXFLAGS:= $(CXXFLAGS) -Iinc
ifeq ("$(MACHTYPE)","sparc-sun-solaris") 
__LIBRARIES := $(__LIBRARIES) -L/opt/EINss7166/lib -leinss7
CXXFLAGS := $(CXXFLAGS) -DUSE_MAPIO
endif
include $(SMSC_SRCDIR)/conf/module_rules
