
#
# $Id$
#
#

PACKAGE := snmp

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CXXFLAGS:= $(CXXFLAGS) -DNETSNMP_NO_INLINE -Iinc

__LIBRARIES := $(__LIBRARIES) -lnetsnmpagent -lnetsnmphelpers -lnetsnmpmibs -lnetsnmp -lsocket -lkstat

include $(SMSC_SRCDIR)/conf/module_rules
