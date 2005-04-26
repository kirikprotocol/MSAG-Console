
#
# $Id$
#
#

PACKAGE := xmap


#CXXFLAGS := -DMICROMAP $(CXXFLAGS)

CXXFLAGS := $(CXXFLAGS) -DUSEPOOL -D_REENTERANT -DDISABLE_ANY_CHECKS

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

#CFLAGS := $(CFLAGS) -I/export/home/green/projects/js/src/js

include $(SMSC_SRCDIR)/conf/module_rules
