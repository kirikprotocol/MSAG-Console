#include <stdio.h>
#include <time.h>
#include "SnmpAppender.hpp"
#include "SnmpAgent.hpp"

namespace smsc {
namespace snmp {

void SnmpAppender::log(const char logLevelName, const char * const category, const char * const message) throw()
{
  agent->trap(message);
}

}
}
