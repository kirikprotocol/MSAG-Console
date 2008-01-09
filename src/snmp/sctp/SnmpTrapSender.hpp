#ident "$Id$"
#ifndef __SMSC_SNMP_SCTP_SNMPTRAPSENDER_HPP__
#define __SMSC_SNMP_SCTP_SNMPTRAPSENDER_HPP__

#include "core/threads/Thread.hpp"
#include "snmp/sctp/TrapSender.hpp"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::core::threads::Thread;
using smsc::snmp::sctp::TrapSender;
class SnmpTrapSender: public Thread, public TrapSender {
public:
  SnmpTrapSender();
  void Stop();
  virtual void run();
  virtual void stop();
  virtual int Execute();
  virtual void trap(SnmpAgent::alertStatus st,const char * const alarmId,const char * const object,SnmpAgent::alertSeverity svr,const char * const text);
};

}//sctp name space
}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SCTP_SNMPTRAPSENDER_HPP__
