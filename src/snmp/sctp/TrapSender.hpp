#ident "$Id$"
#ifndef __SMSC_SNMP_SCTP_TRAPSENDER_HPP__
#define __SMSC_SNMP_SCTP_TRAPSENDER_HPP__

#include "core/threads/Thread.hpp"
#include "snmp/SnmpAgent.hpp"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::core::threads::Thread;
using smsc::snmp::SnmpAgent;
class TrapSender: public Thread{
public:
  TrapSender();
  void Stop();
  virtual int Execute();
  void trap(SnmpAgent::alertStatus st,const char * const alarmId,const char * const object,SnmpAgent::alertSeverity svr,const char * const text);
};

}//sctp name space
}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SCTP_TRAPSENDER_HPP__
