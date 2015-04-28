/*
 * Note: this file originally auto-generated by mbi2c using
 *       version : 1.5.2.2 $ of : subagent.m2c,v $
 */
/* standard Net-SNMP includes */

#define USING_AGENTX_SUBAGENT_MODULE

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "routeErrTable.hpp"

#include <signal.h>


namespace scag2{
namespace snmp{
namespace routeerrtable{

void InitRouteErrTable()
{
  log = smsc::logger::Logger::getInstance("snmp.rerr");
  /* init routeErrTable mib code */
  init_routeErrTable();

  /* read routeErrTable.conf files. */
  init_snmp("routeErrTable");
}

void ShutdownRouteErrTable()
{
  snmp_shutdown("routeErrTable");
}


}//routeErrtable
}//snmp
}//smsc

