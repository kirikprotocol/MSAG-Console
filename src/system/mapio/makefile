
#
# $Id$
#
#

PACKAGE := system.mapio

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CXXFLAGS:= $(CXXFLAGS) -Iinc

ifneq ($(NOMAP),YES)
ifeq ("$(MACHTYPE)","sparc-sun-solaris")
__LIBRARIES := $(__LIBRARIES) -L/opt/EINss7166/lib -leinss7
CXXFLAGS := $(CXXFLAGS) -DUSE_MAP
endif
endif

include $(SMSC_SRCDIR)/conf/module_rules
