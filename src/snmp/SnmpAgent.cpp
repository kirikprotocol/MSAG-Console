static char const ident[] = "$Id$";
#include <net-snmp/net-snmp-config.h>
#include <signal.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "system/smscsignalhandlers.h"
#include "SnmpAgent.hpp"
#include "SnmpAppender.hpp"
#include "TrapRecordLog.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "system/smsc.hpp"
#include "system/version.h"

#include "snmp/smestattable/SmeStatTableSubAgent.hpp"

  extern "C" {
    void init_smsc(void);
    int smscStatusHandler(netsnmp_mib_handler *handler,
         netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
         netsnmp_request_info *requests);
    void sendStatusNotification(unsigned int clientreg, void *clientarg);
    void sendAlarmNotification(unsigned int clientreg, void *clientarg);
    void sendSmscAlertFFMR(unsigned int clientreg, void *clientarg);
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
    void* agent = 0;
    void* agentlog = 0;
    void* smscptr = 0;
    struct timeval  agentStartTime;
    static u_long      accepted = 2;
    static int         status = 0;
    static oid statusNotificationOid[] = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 1 };
    static oid alertOid[] =              { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 2 };
    static oid smscAlertFFMROid[] =      { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 3 };
    static oid smscNewAlertFFMROid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 4 };
    static oid smscClearAlertFFMROid[] = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 5 };
    static oid smscDescrOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 1, 1, 0 };
    static oid status_oid[] =            { 1, 3, 6, 1, 4, 1, 26757, 1, 2, 0 };
    static oid smscUpTimeOid[] =         { 1, 3, 6, 1, 4, 1, 26757, 1, 3, 0 };
    static oid accepted_oid[] =          { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 0 };
    static oid alertMessageOid[] =       { 1, 3, 6, 1, 4, 1, 26757, 1, 5, 0 };
    static oid alertSeverityOid[] =      { 1, 3, 6, 1, 4, 1, 26757, 1, 7, 0 };
    static oid alertIdOid[] =            { 1, 3, 6, 1, 4, 1, 26757, 1, 8, 0 };
    static oid alertObjCategoryOid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 9, 0 };
    static oid sumbitOkOid[] =           { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 1, 0 };
    static oid sumbitErrOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 2, 0 };
    static oid deliverOkOid[] =          { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 3, 0 };
    static oid deliverErrOid[] =         { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 4, 0 };
    static oid rescheduledOid[] =        { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 5, 0 };
  };

  namespace smsc{
    namespace snmp{
      using smsc::system::Smsc;
      using smsc::snmp::TrapRecordLog;
      using smsc::inman::filestore::InFileStorageRoller;
      using smsc::util::config::Manager;
      using smsc::util::config::ConfigException;
      smsc::logger::Logger* SnmpAgent::log = 0;

    struct FFMRargs {
      char* alarmId;
      char* alarmObjCategory;
      char* text;
      uint8_t severity;
      SnmpAgent::alertStatus status;
    };

      TrapRecordLog*       fileLog = 0;
      InFileStorageRoller* fileLogRoller = 0;
      const char *         fileLogLocation = ".";
      unsigned long        fileLogRollInterval = 60;

      const char* SnmpAgent::taskName()
      {
        return "SnmpAgent";
      }

      SnmpAgent::SnmpAgent(Smsc* _smsc)
      {
        log = smsc::logger::Logger::getInstance("sms.snmp");
        smsc::logger::Logger *tlog=smsc::logger::Logger::getInstance("sms.snmp.alarm");//TODO implement SNMP Appender
        tlog->setAppender(new SnmpAppender("-",this));
        agentlog = (void*)log;
        agent = (void*)this;
        smsc = _smsc;
        smscptr = (void*)_smsc;
        init();
      }
      SnmpAgent::~SnmpAgent()
      {
        agent = 0;
        smscptr = 0;
        agentlog = 0;
        if (fileLog)
        {
          fileLog->RFSClose();
          if (fileLogRoller)
          {
            delete fileLogRoller;
            fileLogRoller = 0;
          }
          delete fileLog;
          fileLog = 0;
        }
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
        smestattable::SmeStatTableSubagent::Register();
      }

      int SnmpAgent::Execute()
      {
        Manager& manager = Manager::getInstance();
        try
        {
          fileLogLocation = manager.getString("snmp.csvFileDir");
        }
        catch (ConfigException& exc)
        {
            fileLogLocation=".";
        }

        try
        {
          fileLogRollInterval = manager.getInt("snmp.csvFileRollInterval");
        }
        catch (ConfigException& exc)
        {
            fileLogRollInterval = 60;
        }

        char buf1[64];
        sprintf(buf1,"n%d.ucs.lst",smsc->nodeIndex);
        char buf2[64];
        sprintf(buf2,"n%d.ucs.csv",smsc->nodeIndex);

        fileLog = new TrapRecordLog(fileLogLocation, smsc->nodeIndex?buf1:"ucs.lst",smsc->nodeIndex?buf2:"ucs.csv", fileLogRollInterval, log);
        fileLog->RFSOpen(true);
        fileLogRoller = new InFileStorageRoller(fileLog,fileLogRollInterval);
//        fileLogRoller->attachRFS(fileLog,fileLogRollInterval);

        if (fileLogRoller) {
            fileLogRoller->Start();
        }
        while(!isStopping)
        {
          agent_check_and_process(1);
        }
        if (fileLogRoller) {
            fileLogRoller->Stop();
//            fileLogRoller->WaitFor();
        }
        smsc_log_debug(log, "try to shutdown snmp agent");
        smestattable::SmeStatTableSubagent::Unregister();
        snmp_shutdown("smscd");// at shutdown time
        smsc_log_debug(log, "snmp agent shutdowned");
        return 0;
      }
      void SnmpAgent::statusChange(smscStatus newstatus)
      {
        status = newstatus;
        int *statusSave;
        memdup((uchar_t **) &statusSave,(uchar_t *) &status,sizeof(status));
        struct timeval t;t.tv_sec=0,t.tv_usec=10;
        snmp_alarm_register_hr(t, 0, sendStatusNotification, (void*)statusSave);
        smsc_log_debug(log, "smsc status changed to %d, trap sent, saved = %d(%d)",newstatus,*statusSave,sizeof(status));
      }

      void SnmpAgent::trap(const char * const message)
      {
        TrapRecord rec;
        rec.submitTime = time(0);
        rec.alarmId = "empty";
        rec.alarmObjCategory = "empty";
        rec.severity = NORMAL;
        rec.text = message;
        if(fileLog) fileLog->log(rec);
        struct timeval t;t.tv_sec=0,t.tv_usec=10;
        snmp_alarm_register_hr(t, 0, sendAlarmNotification, strdup(message));
      }
      void SnmpAgent::trap(const char * const alarmId,
                           const char * const alarmObjCategory,
                           alertSeverity severity,
                           const char * const text)
      {
        trap(INFO,alarmId,alarmObjCategory,severity,text);
      }
      void SnmpAgent::trap(alertStatus status,
                           const char * const alarmId,
                           const char * const alarmObjCategory,
                           alertSeverity severity,
                           const char * const text)
      {
        if (!agent)
        {
          smsc_log_warn(log,"snmp agent isn't initialized, trap can't be sent");
          return;
        }
        TrapRecord rec;
        rec.submitTime = time(0);
        rec.alarmId = alarmId;
        rec.alarmObjCategory = alarmObjCategory;
        rec.severity = severity;
        rec.text = text;
        if(fileLog) fileLog->log(rec);
        struct timeval t;t.tv_sec=0,t.tv_usec=10;
        FFMRargs* clientarg = new(FFMRargs);
        if (clientarg)
        {
          clientarg->alarmId = strdup(alarmId);
          clientarg->alarmObjCategory = strdup(alarmObjCategory);
          clientarg->severity = severity;
          clientarg->text = strdup(text);
          clientarg->status = status;
          unsigned int reg_id;
          reg_id = snmp_alarm_register_hr(t, 0 /* once */, sendSmscAlertFFMR,clientarg);
          smsc_log_debug(log,
                         "trap(alarmId=%s,object=%s,severity=%d,text=%s) has been registered to send, registration_id=%X",
                         clientarg->alarmId,
                         clientarg->alarmObjCategory,
                         clientarg->severity,
                         clientarg->text,
                         reg_id);
        }
      }

    }//snmp name space
  }//smsc name space


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
        //MTS dosn't need to stop by SNMP so just send error and do nothing
        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
        break;
        //MTS dosn't need to stop by SNMP so the following code becomes unreached

        smsc_log_debug( ((smsc::logger::Logger*)agentlog), "hello from smscStatusHandler MODE_SET_ACTION");
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
using smsc::snmp::FFMRargs;
using smsc::snmp::SnmpAgent;
//using smsc::snmp::CLEAR;
//using smsc::snmp::INFO;
  /* order of vars in clientarg array: alarmId, alarmObCategory, severity, text */
  extern "C"
  void
  sendSmscAlertFFMR(unsigned int clientreg, void *clientarg)
  {
    FFMRargs* vars = (FFMRargs *)clientarg;
    smsc_log_debug( smsc::snmp::SnmpAgent::log,
                   "trap(alarmId=%s,object=%s,severity=%d,text=%s) is called to send, registration_id=%X",
                   vars->alarmId,
                   vars->alarmObjCategory,
                   vars->severity,
                   vars->text,
                   clientreg);

    if (vars)
    {
      netsnmp_variable_list *notification_vars = NULL;
      u_char* buf; int buflen;
      if (vars->text)
      {
        buf = (u_char*)vars->text;buflen = strlen(vars->text);
        snmp_varlist_add_variable(&notification_vars,
                                  alertMessageOid, OID_LENGTH(alertMessageOid), ASN_OCTET_STR,
                                  buf,buflen);
        free(vars->text);
      }

      {
        buf = &(vars->severity); buflen = sizeof(vars->severity);
        snmp_varlist_add_variable(&notification_vars,
                                  alertSeverityOid, OID_LENGTH(alertSeverityOid), ASN_INTEGER,
                                  buf,buflen);
      }

      if (vars->alarmObjCategory)
      {
        buf = (u_char*)vars->alarmObjCategory;buflen = strlen(vars->alarmObjCategory);
        snmp_varlist_add_variable(&notification_vars,
                                  alertObjCategoryOid, OID_LENGTH(alertObjCategoryOid), ASN_OCTET_STR,
                                  buf,buflen);
        free(vars->alarmObjCategory);
      }

      if (vars->alarmId)
      {
        buf = (u_char*)vars->alarmId;buflen = strlen(vars->alarmId);
        snmp_varlist_add_variable(&notification_vars,
                                  alertIdOid, OID_LENGTH(alertIdOid), ASN_OCTET_STR,
                                  buf,buflen);
        free(vars->alarmId);
      }
      oid* poid = 0;
      int  oidlen = 0;
      switch (vars->status)
      {
        case SnmpAgent::INFO : poid = smscAlertFFMROid; oidlen = OID_LENGTH(smscAlertFFMROid); break;
        case SnmpAgent::NEW  : poid = smscNewAlertFFMROid; oidlen = OID_LENGTH(smscNewAlertFFMROid); break;
        case SnmpAgent::CLEAR: poid = smscClearAlertFFMROid; oidlen = OID_LENGTH(smscClearAlertFFMROid); break;
      }
      send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1, poid, oidlen, notification_vars);
      snmp_free_varbind(notification_vars);
      delete(clientarg);
      smsc_log_debug(smsc::snmp::SnmpAgent::log,"trap with registration_id=%X has been sent", clientreg);
    }
  }

  static char version[] ="SMSC UNKNOWN VERSION";
  extern "C" int smscDescrHandler(netsnmp_mib_handler *handler,
                                  netsnmp_handler_registration *reginfo,
                                  netsnmp_agent_request_info *reqinfo,
                                  netsnmp_request_info *requests)
  {

    switch (reqinfo->mode) {
      case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *)getStrVersion(), strlen(getStrVersion()));
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
    uint64_t perf[6];
    ((smsc::system::Smsc*)smscptr)->getPerfData(perf);

    switch (reqinfo->mode)
    {
      case MODE_GET:
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "hello from stats handler");
        for(i = 0; i <= reginfo->rootoid_len; i++ )
        {
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "oid[%d] = %d",i,reginfo->rootoid[i]);
        }
        if (snmp_oid_compare(sumbitOkOid,OID_LENGTH(sumbitOkOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[0] >> 32;
          val.low  = perf[0] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
         smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitOK req");
        }
        else if (snmp_oid_compare(sumbitErrOid,OID_LENGTH(sumbitErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[1] >> 32;
          val.low  = perf[1] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitErr req");
        }
        else if (snmp_oid_compare(deliverOkOid,OID_LENGTH(deliverOkOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[2] >> 32;
          val.low  = perf[2] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "delivwerOK req");
        }
        else if (snmp_oid_compare(deliverErrOid,OID_LENGTH(deliverErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[3] >> 32;
          val.low  = perf[4] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
        }
        else if (snmp_oid_compare(rescheduledOid,OID_LENGTH(rescheduledOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          val.high = perf[5] >> 32;
          val.low  = perf[5] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "rescheduled req");
        }
        else
        {
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "compate does not work");
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
