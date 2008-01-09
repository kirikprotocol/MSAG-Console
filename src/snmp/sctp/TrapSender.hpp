#ident "$Id$"
#ifndef __SMSC_SNMP_SCTP_TRAPSENDER_HPP__
#define __SMSC_SNMP_SCTP_TRAPSENDER_HPP__

#include "snmp/SnmpAgent.hpp"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::snmp::SnmpAgent;
class TrapSender {
  public:
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void trap(SnmpAgent::alertStatus st,const char * const alarmId,const char * const object,SnmpAgent::alertSeverity svr,const char * const text) = 0;
};

}//sctp name space
}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SCTP_TRAPSENDER_HPP__
