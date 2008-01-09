#ident "$Id$"
#ifndef __SMSC_SNMP_SCTP_NULLTRAPSENDER_HPP__
#define __SMSC_SNMP_SCTP_NULLTRAPSENDER_HPP__
#include "snmp/sctp/TrapSender.hpp"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::snmp::SnmpAgent;
using smsc::snmp::sctp::TrapSender;
class NullTrapSender: public TrapSender {
public:
  NullTrapSender();
  virtual void run();
  virtual void stop();
  virtual void trap(SnmpAgent::alertStatus st,const char * const alarmId,const char * const object,SnmpAgent::alertSeverity svr,const char * const text);
};

}//sctp name space
}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SCTP_NULLTRAPSENDER_HPP__
