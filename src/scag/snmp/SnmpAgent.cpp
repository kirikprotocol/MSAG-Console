#include <signal.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "logger/Logger.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
//#include "smsc/smscsignalhandlers.h"
//#include "smsc/smsc.hpp"
//#include "smsc/version.h"

//#include "util/config/Manager.h"

#include "smestattable/smeStatTable_subagent.hpp"
#include "smeerrtable/smeErrTable_subagent.hpp"
#include "SnmpAgent.hpp"
//#include "SnmpAppender.hpp"
//#include "TrapRecordLog.hpp"
#include <signal.h>
#include "scag/version.h"

static char const ident[] = "$Id$";

extern "C" {
  void init_handlers(void);

  typedef int msagNetSnmpHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo, netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests);

  msagNetSnmpHandler statusHandler;
  msagNetSnmpHandler descrHandler;
  msagNetSnmpHandler uptimeHandler;
  msagNetSnmpHandler statisticsHandler;

//  void sendStatusNotification(unsigned int clientreg, void* clientarg);
//  void sendAlarmNotification(unsigned int clientreg, void* clientarg);

//  void send_change_status();
  void* agent = 0;
  void* agentlog = 0;

  struct timeval  agentStartTime;
  static int     status = 0;
  static time_t  statusTime;
//                                       1.3.6.1.4.1.26757.2.1
  static oid msagDescrOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 2, 1, 0 };
  static oid status_oid[] =            { 1, 3, 6, 1, 4, 1, 26757, 2, 2, 0 };
  static oid uptimeOid[] =             { 1, 3, 6, 1, 4, 1, 26757, 2, 3, 0 };

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

/*
  const char* SnmpAgent::taskName()
  {
    return "SnmpAgent";
  }
*/
  SnmpAgent::SnmpAgent(unsigned node_number)
  {
    node = node_number;
    log = smsc::logger::Logger::getInstance("snmp.agent");
//    smsc::logger::Logger *tlog = smsc::logger::Logger::getInstance("sms.snmp.alarm");//TODO implement SNMP Appender
//    tlog->setAppender(new SnmpAppender("-",this));
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

//    snmp_disable_stderrlog();  // print log errors to stderr
//    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);  // we are a subagent
//    init_agent("msagd");  // initialize the agent library
    smsc_log_debug(log, "before init SNMP handlers");
    try
    {
      init_handlers();      // initialize mib code here
      smsc_log_debug(log, "SNMP handlers init OK");
    }
    catch(...)
    {
      smsc_log_debug(log, "SNMP handlers init failure");
    }
//    init_snmp("msag");    //read .conf files

    smestattable::SmeStatTableSubagent::Register();
    smeerrtable::InitSmeErrTable();
  }
/*
  int SnmpAgent::Execute()
  {
    char buf1[64];
    sprintf(buf1, "n%d.ucs.lst", node);
    char buf2[64];
    sprintf(buf2, "n%d.ucs.csv", node);

    struct timeval t;t.tv_sec=0,t.tv_usec=500000;
    snmp_alarm_register_hr(t, SA_REPEAT, dummyAlert, 0);

    while(!isStopping)
    {
      agent_check_and_process(1);
    }

    smsc_log_debug(log, "try to shutdown snmp agent");
    smestattable::SmeStatTableSubagent::Unregister();
    smeerrtable::ShutdownSmeErrTable();
    snmp_shutdown("msagd");// at shutdown time
    smsc_log_debug(log, "snmp agent shutdowned");
    return 0;
  }
*/
  void SnmpAgent::shutdown()
  {
    smsc_log_debug(log, "try to shutdown snmp agent");
    smestattable::SmeStatTableSubagent::Unregister();
    smeerrtable::ShutdownSmeErrTable();
//    snmp_shutdown("msagd");// at shutdown time
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

    reginfo = netsnmp_create_handler_registration("deliverERR", statisticsHandler, deliverGwErrOid, OID_LENGTH(deliverGwErrOid), HANDLER_CAN_RONLY);
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

    reginfo = netsnmp_create_handler_registration("msagUpTime", uptimeHandler, uptimeOid, OID_LENGTH(uptimeOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    DEBUGMSGTL(("msagUpTime"," handler inited\n"));
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
    DEBUGMSGTL(("descrHandler","called\n"));
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
  int statisticsHandler(netsnmp_mib_handler* handler, netsnmp_handler_registration* reginfo,
                        netsnmp_agent_request_info* reqinfo, netsnmp_request_info* requests)
  {
    int i;
    //netsnmp_variable_list *var = requests->requestvb;
    struct counter64 val;
    uint64_t perf[10];  //fake
//    uint64_t perf[smsc::performanceCounters];
//    ((smsc::Smsc*)smscptr)->getPerfData(perf);

    const int perfBase=6;

    if ( MODE_GET == reqinfo->mode )
    {
//  case MODE_GET:

      smsc_log_debug(((smsc::logger::Logger*)agentlog), "hello from stats handler");
      for( i = 0; i <= reginfo->rootoid_len; i++ )
      {
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "oid[%d] = %d", i, reginfo->rootoid[i]);
      }

#define requestedOidIs(x) (snmp_oid_compare(x, OID_LENGTH(x), reginfo->rootoid, reginfo->rootoid_len) == 0)

      if ( requestedOidIs(sumbitOkOid) )
      {
        val.high = perf[perfBase+0] >> 32;
        val.low  = perf[perfBase+0] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitOK req");
      }
      else if ( requestedOidIs(sumbitErrOid) )
      {
        val.high = perf[perfBase+1] >> 32;
        val.low  = perf[perfBase+1] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitErr req");
      }
      else if ( requestedOidIs(deliverOkOid) )
      {
        val.high = perf[perfBase+2] >> 32;
        val.low  = perf[perfBase+2] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char*)&val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverOK req");
      }
      else if ( requestedOidIs(deliverGwErrOid) )
      {
        val.high = perf[perfBase+3] >> 32;
        val.low  = perf[perfBase+3] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
      }
      else if ( requestedOidIs(deliverErrOid) )
      {
        val.high = perf[perfBase+4] >> 32;
        val.low  = perf[perfBase+4] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
      }
      else if ( requestedOidIs(receiptOkOid) )
      {
        val.high = perf[perfBase+5] >> 32;
        val.low  = perf[perfBase+5] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "rescheduled req");
      }
      else if ( requestedOidIs(receiptErrOid) )
      {
        val.high = perf[perfBase+6] >> 32;
        val.low  = perf[perfBase+6] & 0xffffffff;
        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverTEMP req");
      }
      else
      {
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "OID compare: found nothing");
        netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
      }
    }
    else
    {
/*
 *    case MODE_SET_RESERVE1:
      case MODE_SET_RESERVE2:
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_UNDO:
      case MODE_SET_FREE:
      case MODE_GETNEXT: do nothing
 */
    }
#undef requestedOidIs

    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }
//=============
