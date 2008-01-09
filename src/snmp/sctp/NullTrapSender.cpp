static char const ident[] = "$Id$";
#include "snmp/sctp/NullTrapSender.hpp"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::logger::Logger;
static Logger* logger = 0;

void NullTrapSender::run() {}
void NullTrapSender::stop() {}

NullTrapSender::NullTrapSender()
{
  logger = Logger::getInstance("sctp.trap");
}

void NullTrapSender::trap(SnmpAgent::alertStatus st,
                      const char * const alarmId,
                      const char * const object,
                      SnmpAgent::alertSeverity svr,
                      const char * const text)
{
  smsc_log_debug(logger,"NullTrapSender sent a trap");
}
}//namespace sctp
}//namespace snmp
}//namespace smsc
