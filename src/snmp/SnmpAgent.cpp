#include <net-snmp/net-snmp-config.h>
#include <signal.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "system/smscsignalhandlers.h"
#include "SnmpAgent.hpp"
#include "SnmpAppender.hpp"
#include "logger/Logger.h"
#include "system/smsc.hpp"
  extern "C" {
    void init_smsc(void);
    int smscStatusHandler(netsnmp_mib_handler *handler,
				 netsnmp_handler_registration *reginfo,
		                 netsnmp_agent_request_info *reqinfo,
				 netsnmp_request_info *requests);
    void sendStatusNotification(unsigned int clientreg, void *clientarg);
    void sendAlarmNotification(unsigned int clientreg, void *clientarg);
    int smscDescrHandler(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests);
    int smscUpTimeHandler(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests);
    int smscStatsHandler(netsnmp_mib_handler *handler,
                         netsnmp_handler_registration *reginfo,
                         netsnmp_agent_request_info *reqinfo,
                         netsnmp_request_info *requests);
    void send_change_status();
    void* agent;
    void* agentlog;
    void* smscptr;
    struct timeval  agentStartTime;
    static u_long      accepted = 2;
    static int         status = 0;
    static oid statusNotificationOid[] = { 1, 3, 6, 1, 4, 1, 17939, 1, 0, 1 };
    static oid alertOid[] =              { 1, 3, 6, 1, 4, 1, 17939, 1, 0, 2 };
    static oid smscDescrOid[] =          { 1, 3, 6, 1, 4, 1, 17939, 1, 1, 0 };
    static oid status_oid[] =            { 1, 3, 6, 1, 4, 1, 17939, 1, 2, 0 };
    static oid smscUpTimeOid[] =         { 1, 3, 6, 1, 4, 1, 17939, 1, 3, 0 };
    static oid accepted_oid[] =          { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 0 };
    static oid alertMessageOid[] =       { 1, 3, 6, 1, 4, 1, 17939, 1, 5, 0 };
    static oid sumbitOkOid[] =           { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 1, 0 };
    static oid sumbitErrOid[] =          { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 2, 0 };
    static oid deliverOkOid[] =          { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 3, 0 };
    static oid deliverErrOid[] =         { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 4, 0 };
    static oid rescheduledOid[] =        { 1, 3, 6, 1, 4, 1, 17939, 1, 4, 5, 0 };
  };

  namespace smsc{
    namespace snmp{
      using smsc::system::Smsc;

      const char* SnmpAgent::taskName() 
      { 
        return "SnmpAgent";
      }

      SnmpAgent::SnmpAgent(Smsc* _smsc)
      {
        log=&smsc::util::Logger::getCategory("sms.snmp");
        log4cpp::Category& tlog=smsc::util::Logger::getCategory("sms.snmp.alarm");
        tlog.addAppender(new SnmpAppender("-",this));
        agentlog = (void*)log;
        agent = (void*)this;
        smsc = _smsc;
        smscptr = (void*)_smsc;
        init();
      }
      SnmpAgent::~SnmpAgent()
      {
        log->debug( "destructor" );
        agent = 0;
        smscptr = 0;
        agentlog = 0;
      }

      void SnmpAgent::init()
      {
        gettimeofday(&agentStartTime, NULL);
        agentStartTime.tv_sec--;
        agentStartTime.tv_usec += 1000000L;
        snmp_disable_stderrlog();  // print log errors to stderr
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                               NETSNMP_DS_AGENT_ROLE, 1);  // we are a subagent
        init_agent("smscd");  // initialize the agent library
        init_smsc();          // initialize mib code here 
        init_snmp("smsc");    //read .conf files
      }

      int SnmpAgent::Execute()
      {
         while(!isStopping)
         {
           agent_check_and_process(1);
         }
         log->debug("try to shutdown snmp agent");
         snmp_shutdown("smscd");// at shutdown time
         log->debug("snmp agent shutdowned");
         return 0;
      }
      void SnmpAgent::statusChange(smscStatus newstatus)
      {
        status = newstatus;
        int *statusSave;
        memdup((uchar_t **) &statusSave,(uchar_t *) &status,sizeof(status));
        struct timeval t;t.tv_sec=0,t.tv_usec=10000;
        snmp_alarm_register_hr(t, 0, sendStatusNotification, (void*)statusSave);
        log->debug("smsc status changed to %d, trap sent, saved = %d(%d)",newstatus,*statusSave,sizeof(status));
      }

      void SnmpAgent::trap(std::string &message)
      {
        struct timeval t;t.tv_sec=0,t.tv_usec=10000;
        snmp_alarm_register_hr(t, 0, sendAlarmNotification, strdup(message.c_str()));
      }

    };//snmp name space
  };//smsc name space


  extern "C"
  void
  init_smsc(void)
  {
    netsnmp_handler_registration *reginfo;
    reginfo = netsnmp_create_handler_registration("statusOK", smscStatsHandler, sumbitOkOid, OID_LENGTH(sumbitOkOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("statusERR", smscStatsHandler, sumbitErrOid, OID_LENGTH(sumbitErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("deliverOk", smscStatsHandler, deliverOkOid, OID_LENGTH(deliverOkOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("deliverERR", smscStatsHandler, deliverErrOid, OID_LENGTH(deliverErrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("rescheduled", smscStatsHandler, rescheduledOid, OID_LENGTH(rescheduledOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("status", smscStatusHandler, status_oid, OID_LENGTH(status_oid), HANDLER_CAN_RWRITE);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("smscDescr", smscDescrHandler, smscDescrOid, OID_LENGTH(smscDescrOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("smscUpTime", smscUpTimeHandler, smscUpTimeOid, OID_LENGTH(smscUpTimeOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
  }

  extern "C"
  int
  smscStatusHandler(netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
  {

    int            *it = &status;
    int            *it_save;

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
        //*it = (int) *(requests->requestvb->val.integer);
        ((log4cpp::Category*)agentlog)->debug("hello from smscStatusHandler MODE_SET_ACTION");
        if (agent)
        {
          if (status != smsc::snmp::SnmpAgent::OPER)
          {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
          }
          else
          {
            if (*(requests->requestvb->val.integer) != smsc::snmp::SnmpAgent::TERM)
            {
              netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
            }
            else
            {
              kill(getpid(),smsc::system::SHUTDOWN_SIGNAL);
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

  extern "C"
  void
  sendStatusNotification(unsigned int clientreg, void *clientarg)
  {
    netsnmp_variable_list *notification_vars = NULL;
    snmp_varlist_add_variable(&notification_vars,
                              status_oid, OID_LENGTH(status_oid), ASN_INTEGER,
                              (u_char *) clientarg, sizeof(status));
    send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                              statusNotificationOid,
                              OID_LENGTH(statusNotificationOid), notification_vars);
    snmp_free_varbind(notification_vars);
    free(clientarg);
  }

  extern "C"
  void
  sendAlarmNotification(unsigned int clientreg, void *clientarg)
  {
    netsnmp_variable_list *notification_vars = NULL;
    snmp_varlist_add_variable(&notification_vars,
                              alertMessageOid, OID_LENGTH(alertMessageOid), ASN_OCTET_STR,
                              (u_char *) clientarg, strlen((const char *)clientarg));
    send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                              alertOid,
                              OID_LENGTH(alertOid), notification_vars);
    snmp_free_varbind(notification_vars);
    free(clientarg);
  }
  static char version[] ="SMSC UNKNOWN VERSION";
  extern "C" int smscDescrHandler(netsnmp_mib_handler *handler,
                                  netsnmp_handler_registration *reginfo,
                                  netsnmp_agent_request_info *reqinfo,
                                  netsnmp_request_info *requests)
  {

    switch (reqinfo->mode) {
      case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *)version, strlen(version));
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
  int smscUpTimeHandler(netsnmp_mib_handler *handler,
                        netsnmp_handler_registration *reginfo,
                        netsnmp_agent_request_info *reqinfo,
                        netsnmp_request_info *requests)
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
  int smscStatsHandler(netsnmp_mib_handler *handler,
                                netsnmp_handler_registration *reginfo,
                                netsnmp_agent_request_info *reqinfo,
                                netsnmp_request_info *requests)
  {

    int i;
    netsnmp_variable_list *var = requests->requestvb;
    struct counter64 val;
    uint64_t perf[5];
    ((smsc::system::Smsc*)smscptr)->getPerfData(perf);

    switch (reqinfo->mode)
    {
      case MODE_GET:
        ((log4cpp::Category*)agentlog)->debug("hello from stats handler");
        for(i = 0; i <= reginfo->rootoid_len; i++ )
        {
        ((log4cpp::Category*)agentlog)->debug("oid[%d] = %d",i,reginfo->rootoid[i]);
        }
        if (snmp_oid_compare(sumbitOkOid,OID_LENGTH(sumbitOkOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[0] >> 32;
          val.low  = perf[0] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
         ((log4cpp::Category*)agentlog)->debug("submitOK req");
        }
        else if (snmp_oid_compare(sumbitErrOid,OID_LENGTH(sumbitErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[1] >> 32;
          val.low  = perf[1] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        ((log4cpp::Category*)agentlog)->debug("submitErr req");
        }
        else if (snmp_oid_compare(deliverOkOid,OID_LENGTH(deliverOkOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[2] >> 32;
          val.low  = perf[2] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        ((log4cpp::Category*)agentlog)->debug("delivwerOK req");
        }
        else if (snmp_oid_compare(deliverErrOid,OID_LENGTH(deliverErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[3] >> 32;
          val.low  = perf[4] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        ((log4cpp::Category*)agentlog)->debug("deliverERR req");
        }
        else if (snmp_oid_compare(rescheduledOid,OID_LENGTH(rescheduledOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          val.high = perf[5] >> 32;
          val.low  = perf[5] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        ((log4cpp::Category*)agentlog)->debug("rescheduled req");
        }
        else 
        {
        ((log4cpp::Category*)agentlog)->debug("compate does not work");
          netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
        }
        return SNMP_ERR_NOERROR;
      case MODE_SET_RESERVE1:
      case MODE_SET_RESERVE2:
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_UNDO:
      case MODE_SET_FREE:
      case MODE_GETNEXT:
        break;
    }
    if (handler->next && handler->next->access_method)
    {
      return netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
    }
    return SNMP_ERR_NOERROR;
  }
