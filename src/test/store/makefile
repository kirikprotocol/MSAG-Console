CXXFLAGS := $(CXXFLAGS) -I$(SMSC_SRCDIR)/sms -I$(SMSC_SRCDIR)/store -I/usr/local/include/xercesc -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/network/public
PACKAGE := test.store

__LIBRARIES :=  $(ORACLE_HOME)/lib/libclntsh.so
__OTHER := -L$(ORACLE_HOME)/lib -lstore -lcore-threads -lcore-buffers -lsms -lutil-config -lutil-xml -lutil -llog4cpp -lxerces-c $(__LIBRARIES) -lthread -lrt

ifndef SMSC_SRCDIR
$(error SMSC_SRCDIR is not defined)
endif

include $(SMSC_SRCDIR)/conf/module_rules

