
#
# $Id$
#
#

CXXFLAGS := $(CXXFLAGS) -I.
PACKAGE := sua.libsua.messages

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CPPFLAGS := -D_POSIX_PTHREAD-SEMANTICS -D_XOPEN_SOURCE=500
include $(SMSC_SRCDIR)/conf/module_rules
