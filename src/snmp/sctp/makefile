
#
# $Id$
#
#

PACKAGE := snmp.sctp

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

CXXFLAGS:= $(CXXFLAGS) -DNETSNMP_NO_INLINE -Iinc

__LIBRARIES := $(__LIBRARIES) -lnetsnmpagent -lnetsnmphelpers -lnetsnmpmibs -lnetsnmp -lsocket -lkstat -lsnmp -L$(SS7_STACK_HOME)/lib

include $(SMSC_SRCDIR)/conf/module_rules
