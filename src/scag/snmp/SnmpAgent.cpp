#include <signal.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "logger/Logger.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "scag/counter/Manager.h"

#include "smestattable/smeStatTable_subagent.hpp"
#include "smeerrtable/smeErrTable_subagent.hpp"
#include "routestattable/routeStatTable_subagent.hpp"
#include "routeerrtable/routeErrTable_subagent.hpp"
#include "scag/stat/impl/StatCountersEnum.hpp"
#include "SnmpAgent.hpp"
#include <signal.h>
#include "scag/version.h"

static char const ident[] = "$Id$";

extern "C" {
  void init_handlers(void);

  typedef int msagNetSnmpHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo, netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests);

  msagNetSnmpHandler statusHandler;
  msagNetSnmpHandler descrHandler;
  msagNetSnmpHandler uptimeHandler;
  msagNetSnmpHandler uptimeFormattedHandler;
  msagNetSnmpHandler statisticsHandler;

  void* agent = 0;
  void* agentlog = 0;

  struct timeval  agentStartTime;
  static int     status = 0;
  static time_t  statusTime;

  static oid msagDescrOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 2, 1 };
  static oid status_oid[] =            { 1, 3, 6, 1, 4, 1, 26757, 2, 2 };
  static oid uptimeOid[] =             { 1, 3, 6, 1, 4, 1, 26757, 2, 3 };
  static oid uptimeOid1[] =            { 1, 3, 6, 1, 4, 1, 26757, 2, 3, 0 };

  static oid sumbitOkOid[] =           { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 3, 0 };
  static oid sumbitErrOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 4, 0 };
  static oid deliverOkOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 5, 0 };
  static oid deliverGwErrOid[] =       { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 6, 0 };
  static oid deliverErrOid[] =         { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 7, 0 };
  static oid receiptOkOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 8, 0 };
  static oid receiptErrOid[] =         { 1, 3, 6, 1, 4, 1, 26757, 2, 4, 9, 0 };
};

  namespace scag2 {
  namespace snmp {

  smsc::logger::Logger* SnmpAgent::log = 0;
  using namespace smsc::core::buffers;

  extern "C" void dummyAlert(unsigned,void*)
  {
  }

  SnmpAgent::SnmpAgent(unsigned node_number)
  {
    node = node_number;
    log = smsc::logger::Logger::getInstance("snmp.agent");
    agentlog = (void*)log;
    agent = (void*)this;
    init();
  }

  SnmpAgent::~SnmpAgent()
  {
    shutdown();
    agent = 0;
    agentlog = 0;
  }

  void SnmpAgent::init()
  {
    gettimeofday(&agentStartTime, NULL);
    agentStartTime.tv_sec--;
    agentStartTime.tv_usec += 1000000L;

    try
    {
      init_handlers();      // initialize mib code here
    }
    catch(...)
    {
      smsc_log_error(log, "SNMP handlers init failure");
    }

    smestattable::SmeStatTableSubagent::Register();
    smeerrtable::InitSmeErrTable();
    routestattable::RouteStatTableSubagent::Register();
    routeerrtable::InitRouteErrTable();
  }

  void SnmpAgent::shutdown()
  {
    smsc_log_debug(log, "try to shutdown snmp agent");
    smestattable::SmeStatTableSubagent::Unregister();
    smeerrtable::ShutdownSmeErrTable();
    routestattable::RouteStatTableSubagent::Unregister();
    routeerrtable::ShutdownRouteErrTable();
    smsc_log_debug(log, "snmp agent shutdowned");
  }

  void SnmpAgent::statusChange(msagStatus newstatus)
  {
    statusTime = time(NULL);
    status = newstatus;
    smsc_log_debug(log, "msag status changed to %d", newstatus);
  }


} } //namespace


  extern "C"
  void
  init_handlers(void)
  {
    netsnmp_handler_registration *reginfo;
    reginfo = netsnmp_create_handler_registration("statusOK", statisticsHandler, sumbitOkOid, OID_LENGTH(sumbitOkOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("statusOK"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("statusERR", statisticsHandler, sumbitErrOid, OID_LENGTH(sumbitErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("statusERR"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("deliverOk", statisticsHandler, deliverOkOid, OID_LENGTH(deliverOkOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("deliverOk"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("deliverGwERR", statisticsHandler, deliverGwErrOid, OID_LENGTH(deliverGwErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("deliverGwErr"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("deliverERR", statisticsHandler, deliverErrOid, OID_LENGTH(deliverErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("deliverERR"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("receiptOk", statisticsHandler, receiptOkOid, OID_LENGTH(receiptOkOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("receiptOk"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("receiptErr", statisticsHandler, receiptErrOid, OID_LENGTH(receiptErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("receiptErr"," handler inited\n"));


    reginfo = netsnmp_create_handler_registration("status", statusHandler, status_oid, OID_LENGTH(status_oid), HANDLER_CAN_RWRITE);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("status"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("msagDescr", descrHandler, msagDescrOid, OID_LENGTH(msagDescrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("msagDescr"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("msagUptime", uptimeHandler, uptimeOid, OID_LENGTH(uptimeOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("msagUpTime"," handler inited\n"));

    reginfo = netsnmp_create_handler_registration("msagUptimeFormatted", uptimeFormattedHandler, uptimeOid1, OID_LENGTH(uptimeOid1), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("msagUpTimeF"," handler inited\n"));
  }

  extern "C"
  int statusHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                    netsnmp_agent_request_info* reqinfo, netsnmp_request_info *requests)
  {
    int* it = &status;
    int* it_save = 0;

    switch (reqinfo->mode) {
      case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) it, sizeof(*it));
        break;
      case MODE_SET_RESERVE1:
        if (requests->requestvb->type != ASN_INTEGER)
        {
          netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
        }
        break;
      case MODE_SET_RESERVE2:
        memdup((u_char **) & it_save, (u_char *) it, sizeof(u_long));
        if (it_save == NULL)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests, netsnmp_create_data_list(INSTANCE_HANDLER_NAME, it_save, free));
        break;
      case MODE_SET_ACTION:
        //MTS dosn't need to stop by SNMP so just send error and do nothing
        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
        break;
        //MTS dosn't need to stop by SNMP so the following code becomes unreached

        smsc_log_debug( ((smsc::logger::Logger*)agentlog), "hello from statusHandler MODE_SET_ACTION");
        if (agent)
        {
          if (status != scag2::snmp::SnmpAgent::OPER)
          {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
          }
          else
          {
            if (*(requests->requestvb->val.integer) != scag2::snmp::SnmpAgent::TERM)
            {
              netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
            }
            else
            {
              kill(getpid(), SIGTERM);
            }
          }
        }
        break;
      case MODE_SET_UNDO:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE: break;
    }
    if (handler->next && handler->next->access_method)
    {
        return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }

using scag2::snmp::SnmpAgent;
//using smsc::snmp::CLEAR;
//using smsc::snmp::INFO;

  //static char version[] ="MSAG UNKNOWN VERSION";
  extern "C"
  int descrHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                   netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests)
  {
    if ( MODE_GET == reqinfo->mode )
    {
      const char* str_version = getStrVersion();
      snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char*)str_version, strlen(str_version));
    }
/*
    switch (reqinfo->mode)
    {
      case MODE_GET:
        const char* str_version = getStrVersion();
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char*)str_version, strlen(str_version));
        break;
      case MODE_SET_RESERVE1:
      case MODE_SET_RESERVE2:
      case MODE_SET_ACTION:
      case MODE_SET_UNDO:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
        break;
    }
*/
    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }

  extern "C"
  int uptimeHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                    netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests)
  {
    struct timeval diff, now;
    u_long agentUpTime;

    switch (reqinfo->mode) {
      case MODE_GET:
        gettimeofday(&now, NULL);
        diff.tv_sec = now.tv_sec - agentStartTime.tv_sec - 1;
        diff.tv_usec = now.tv_usec - agentStartTime.tv_usec + 1000000;
        agentUpTime = ((u_long) diff.tv_sec) * 100 + diff.tv_usec / 10000;
        snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS, (u_char *)&agentUpTime, sizeof(agentUpTime));
        break;
      case MODE_SET_RESERVE1:
      case MODE_SET_RESERVE2:
      case MODE_SET_ACTION:
      case MODE_SET_UNDO:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
        break;
    }
    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }

  extern "C"
  int uptimeFormattedHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                    netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests)
  {
    struct timeval diff, now;
    u_long days, hours, mins, secs, s100;
    char strTime[64];

    if ( MODE_GET == reqinfo->mode )
    {
      gettimeofday(&now, NULL);
      diff.tv_sec = now.tv_sec - agentStartTime.tv_sec - 1;
      diff.tv_usec = now.tv_usec - agentStartTime.tv_usec + 1000000;
      diff.tv_usec = (diff.tv_usec > 0) ? diff.tv_usec : 0;
      days = diff.tv_sec / 86400;
      diff.tv_sec %= 86400;
      hours = diff.tv_sec / 3600;
      diff.tv_sec %= 3600;
      mins = diff.tv_sec / 60;
      secs = diff.tv_sec % 60;
      s100 = diff.tv_usec / 10000;
      sprintf(strTime, "%d days %02u:%02u:%02u.%02u", days, hours, mins, secs, s100);
      snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char*)strTime, strlen(strTime));
    }
    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }

/*
  extern "C"
  void cntDump(smsc::logger::Logger* lg)
  {
    MsagCounterTableElement *list;

    scag2::counter::Manager::getInstance().updateSnmpCounterList(list);
    if (msagCounterTable_creator) head = (*msagCounterTable_creator)( head );

    for ( list = head; list != NULL; list = list->next )
    {

      if ( !list->enabled ) {
          ++msagCounterIndex;
          continue;
      }
    }
  }
*/

  extern "C"
  void oidLogDump(smsc::logger::Logger* lg, oid* id, size_t oidlen, const char* message)
  {
    char buf[32];
    const char* msg = message ? message : "";
    std::string text = "";
    if ( oidlen ) { snprintf(buf, 32, "%d", id[0]); text = buf; }

    for( int i = 1; i < oidlen; ++i ) { snprintf(buf, 32, ".%u", id[i]); text += buf; }
    smsc_log_debug(lg, "%s oid = [%s]", msg, text.c_str());
  }

  extern "C"
  int statisticsHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                        netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests)
  {
    smsc::logger::Logger* log = (smsc::logger::Logger*)agentlog;
    smsc_log_debug(log, "statisticsHandler info: handlerName=%s contextName=%s",
      reginfo->handlerName?reginfo->handlerName:"default",
      reginfo->contextName?reginfo->contextName:"default");
    oidLogDump(log, reginfo->rootoid, reginfo->rootoid_len, "statisticsHandler info:");

    smsc_log_debug(log, "statisticsHandler info: priority=%d range_subid=%d range_ubound=%u timeout=%d, global_cacheid=%d",
        reginfo->priority, reginfo->range_subid, reginfo->range_ubound, reginfo->timeout, reginfo->global_cacheid);

    struct counter64 val;
    uint64_t perf[scag2::stat::Counters::cntSmppSize];
    scag2::snmp::smestattable::SmeStatTableSubagent::getStatMan()->getSmppPerfData(perf);

    if ( MODE_GET == reqinfo->mode )
    {
      oidLogDump(log, reginfo->rootoid, reginfo->rootoid_len, "hello from stats handler;");
      scag2::counter::Manager::getInstance().dumpCounterList();

#define requestedOidIs(x) (snmp_oid_compare(x, OID_LENGTH(x), reginfo->rootoid, reginfo->rootoid_len) == 0)

      if ( requestedOidIs(sumbitOkOid) )
      {
        val.high = perf[scag2::stat::Counters::cntAccepted] >> 32;
        val.low  = perf[scag2::stat::Counters::cntAccepted] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitOK req");
      }
      else if ( requestedOidIs(sumbitErrOid) )
      {
        val.high = perf[scag2::stat::Counters::cntRejected] >> 32;
        val.low  = perf[scag2::stat::Counters::cntRejected] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitErr req");
      }
      else if ( requestedOidIs(deliverOkOid) )
      {
        val.high = perf[scag2::stat::Counters::cntDelivered] >> 32;
        val.low  = perf[scag2::stat::Counters::cntDelivered] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverOK req");
      }
      else if ( requestedOidIs(deliverGwErrOid) )
      {
        val.high = perf[scag2::stat::Counters::cntGw_Rejected] >> 32;
        val.low  = perf[scag2::stat::Counters::cntGw_Rejected] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
      }
      else if ( requestedOidIs(deliverErrOid) )
      {
        val.high = perf[scag2::stat::Counters::cntFailed] >> 32;
        val.low  = perf[scag2::stat::Counters::cntFailed] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
      }
      else if ( requestedOidIs(receiptOkOid) )
      {
        val.high = perf[scag2::stat::Counters::cntRecieptOk] >> 32;
        val.low  = perf[scag2::stat::Counters::cntRecieptOk] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "receiptOk req");
      }
      else if ( requestedOidIs(receiptErrOid) )
      {
        val.high = perf[scag2::stat::Counters::cntRecieptFailed] >> 32;
        val.low  = perf[scag2::stat::Counters::cntRecieptFailed] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "receiptErr req");
      }
      else
      {
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "OID compare: found nothing");
        netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
      }
    }

#undef requestedOidIs

    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }

