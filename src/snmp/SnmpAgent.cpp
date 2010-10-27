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
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"

#ifdef USE_MAP
#include "system/mapio/MapIoTask.h"
#endif

#include "snmp/smestattable/SmeStatTableSubAgent.hpp"
#include "snmp/smeerrtable/smeErrTable_subagent.hpp"

static char const ident[] = "$Id$";

  extern "C" {
    void init_smsc(void);
    int smscStatusHandler(netsnmp_mib_handler *handler,
         netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
         netsnmp_request_info *requests);
    void sendStatusNotification(unsigned int clientreg, void *clientarg);
    void sendAlarmNotification(unsigned int clientreg, void *clientarg);
    //void sendSmscAlertFFMR(unsigned int clientreg, void *clientarg);
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
    static oid deliverTmpOid[] =         { 1, 3, 6, 1, 4, 1, 26757, 1, 4, 6, 0 };
    static oid mapStatDlgInSRIOid[] =    { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 1 };
    static oid mapStatDlgInOid[] =       { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 2 };
    static oid mapStatDlgOutSRIOid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 3 };
    static oid mapStatDlgOutOid[] =      { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 4 };
    static oid mapStatDlgUSSDOid[] =     { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 5 };
    static oid mapStatDlgNIUSSDOid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 12, 6 };
  };

  namespace smsc{
    namespace snmp{
      using smsc::system::Smsc;
      using smsc::snmp::TrapRecordLog;
      using smsc::inman::filestore::InFileStorageRoller;
      using smsc::util::config::Manager;
      using smsc::util::config::ConfigException;
      smsc::logger::Logger* SnmpAgent::log = 0;
      using smsc::core::buffers;


    CyclicQueue<TrapRecord> trapsQueue;
    smsc::core::synchronization::EventMonitor trapsQueueMon;
    TrapRecordLog*       fileLog = 0;
    InFileStorageRoller* fileLogRoller = 0;
    const char *         fileLogLocation = ".";
    unsigned long        fileLogRollInterval = 60;

    extern "C" void dummyAlert(unsigned,void*)
    {
    }

    void
    ProcessTrap( TrapRecord* vars)
    {
      if(vars->recordType==TrapRecord::rtTrap)
      {
        if(fileLog) fileLog->log(*vars);
        smsc_log_debug( smsc::snmp::SnmpAgent::log,
                       "trap(alarmId=%s,object=%s,severity=%d,text=%s)",
                       vars->alarmId.c_str(),
                       vars->alarmObjCategory.c_str(),
                       vars->severity,
                       vars->text.c_str());

        if (vars)
        {
          netsnmp_variable_list *notification_vars = NULL;
          u_char* buf; size_t buflen;
          if (vars->text.length())
          {
            buf = (u_char*)vars->text.c_str();buflen = vars->text.length();
            snmp_varlist_add_variable(&notification_vars,
                                      alertMessageOid, OID_LENGTH(alertMessageOid), ASN_OCTET_STR,
                                      buf,buflen);
          }

          {
            buf = (u_char*)&(vars->severity); buflen = sizeof(vars->severity);
            snmp_varlist_add_variable(&notification_vars,
                                      alertSeverityOid, OID_LENGTH(alertSeverityOid), ASN_INTEGER,
                                      buf,buflen);
          }

          if (vars->alarmObjCategory.length())
          {
            buf = (u_char*)vars->alarmObjCategory.c_str();buflen = vars->alarmObjCategory.length();
            snmp_varlist_add_variable(&notification_vars,
                                      alertObjCategoryOid, OID_LENGTH(alertObjCategoryOid), ASN_OCTET_STR,
                                      buf,buflen);
          }

          if (vars->alarmId.length())
          {
            buf = (u_char*)vars->alarmId.c_str();buflen = vars->alarmId.length();
            snmp_varlist_add_variable(&notification_vars,
                                      alertIdOid, OID_LENGTH(alertIdOid), ASN_OCTET_STR,
                                      buf,buflen);
          }
          oid* poid = 0;
          size_t  oidlen = 0;
          switch (vars->status)
          {
            case SnmpAgent::INFO : poid = smscAlertFFMROid; oidlen = OID_LENGTH(smscAlertFFMROid); break;
            case SnmpAgent::NEW  : poid = smscNewAlertFFMROid; oidlen = OID_LENGTH(smscNewAlertFFMROid); break;
            case SnmpAgent::CLEAR: poid = smscClearAlertFFMROid; oidlen = OID_LENGTH(smscClearAlertFFMROid); break;
          }
          send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1, poid, (int)oidlen, notification_vars);
          snmp_free_varbind(notification_vars);
        }
      }else if(vars->recordType==TrapRecord::rtStatusChange)
      {
        if(fileLog) fileLog->log(*vars);
        netsnmp_variable_list *notification_vars = NULL;
        snmp_varlist_add_variable(&notification_vars,
                                  status_oid, OID_LENGTH(status_oid), ASN_INTEGER,
                                  (u_char *) &vars->status, sizeof(vars->status));
        send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                                  statusNotificationOid,
                                  (int)OID_LENGTH(statusNotificationOid), notification_vars);
        snmp_free_varbind(notification_vars);
      }else if(vars->recordType==TrapRecord::rtNotification)
      {
        netsnmp_variable_list *notification_vars = NULL;
        snmp_varlist_add_variable(&notification_vars,
                                  alertMessageOid, OID_LENGTH(alertMessageOid), ASN_OCTET_STR,
                                  (u_char *) vars->text.c_str(), vars->text.length());
        send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1,
                                  alertOid,
                                  (int)OID_LENGTH(alertOid), notification_vars);
        snmp_free_varbind(notification_vars);
      }
    }


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
        smeerrtable::InitSmeErrTable();
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
        struct timeval t;t.tv_sec=0,t.tv_usec=500000;
        snmp_alarm_register_hr(t, SA_REPEAT, dummyAlert, 0);

        while(!isStopping)
        {
          agent_check_and_process(1);
          smsc::core::synchronization::MutexGuard mg(trapsQueueMon);
          TrapRecord tr;
          while(trapsQueue.Count()!=0)
          {
            trapsQueue.Pop(tr);
            trapsQueueMon.Unlock();
            try{
              ProcessTrap(&tr);
            }catch(...)
            {
              //
            }
            trapsQueueMon.Lock();
          }
        }
        if (fileLogRoller) {
            fileLogRoller->Stop();
//            fileLogRoller->WaitFor();
        }
        smsc_log_debug(log, "try to shutdown snmp agent");
        smestattable::SmeStatTableSubagent::Unregister();
        smeerrtable::ShutdownSmeErrTable();
        snmp_shutdown("smscd");// at shutdown time
        smsc_log_debug(log, "snmp agent shutdowned");
        return 0;
      }
      void SnmpAgent::statusChange(smscStatus newstatus)
      {
        TrapRecord rec;
        rec.submitTime=time(NULL);
        rec.recordType=TrapRecord::rtStatusChange;
        rec.status=newstatus;
        smsc::core::synchronization::MutexGuard mg(trapsQueueMon);
        trapsQueue.Push(rec);
        //status = newstatus;
        /*int *statusSave;
        memdup((uchar_t **) &statusSave,(uchar_t *) &status,sizeof(status));
        struct timeval t;t.tv_sec=0,t.tv_usec=10;
        snmp_alarm_register_hr(t, 0, sendStatusNotification, (void*)statusSave);
        */
        smsc_log_debug(log, "smsc status changed to %d",newstatus);

      }

      void SnmpAgent::trap(const char * const message)
      {
        TrapRecord rec;
        rec.submitTime=time(NULL);
        rec.recordType=TrapRecord::rtNotification;
        rec.alarmId="empty";
        rec.alarmObjCategory="empty";
        rec.severity=NORMAL;
        rec.text = message;
        //if(fileLog) fileLog->log(rec);
        //struct timeval t;t.tv_sec=0,t.tv_usec=10;
        //snmp_alarm_register_hr(t, 0, sendAlarmNotification, strdup(message));
        smsc::core::synchronization::MutexGuard mg(trapsQueueMon);
        trapsQueue.Push(rec);
      }
      void SnmpAgent::trap(const char * const alarmId,
                           const char * const alarmObjCategory,
                           alertSeverity severity,
                           const char * const text)
      {
        trap(INFO,alarmId,alarmObjCategory,severity,text);
      }
      void SnmpAgent::trap(alertStatus argStatus,
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
        rec.submitTime=time(NULL);
        rec.recordType=TrapRecord::rtTrap;
        rec.alarmId = alarmId;
        rec.alarmObjCategory = alarmObjCategory;
        rec.severity = severity;
        rec.text = text;
        rec.status = argStatus;
        smsc::core::synchronization::MutexGuard mg(trapsQueueMon);
        trapsQueue.Push(rec);
        /*
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
          reg_id = snmp_alarm_register_hr(t, 0 , sendSmscAlertFFMR,clientarg);
          smsc_log_debug(log,
                         "trap(alarmId=%s,object=%s,severity=%d,text=%s) has been registered to send, registration_id=%X",
                         clientarg->alarmId,
                         clientarg->alarmObjCategory,
                         clientarg->severity,
                         clientarg->text,
                         reg_id);
        }
        */
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
    reginfo = netsnmp_create_handler_registration("deliverTEMP", smscStatsHandler, deliverTmpOid, OID_LENGTH(deliverTmpOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);


    reginfo = netsnmp_create_handler_registration("dlgInSRI", smscStatsHandler, mapStatDlgInSRIOid, OID_LENGTH(mapStatDlgInSRIOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("dlgIn", smscStatsHandler, mapStatDlgInOid, OID_LENGTH(mapStatDlgInOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("dlgOutSRI", smscStatsHandler, mapStatDlgOutSRIOid, OID_LENGTH(mapStatDlgOutSRIOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("dlgOut", smscStatsHandler, mapStatDlgOutOid, OID_LENGTH(mapStatDlgOutOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("dlgUSSD", smscStatsHandler, mapStatDlgUSSDOid, OID_LENGTH(mapStatDlgUSSDOid), HANDLER_CAN_RONLY);
    netsnmp_register_instance(reginfo);
    reginfo = netsnmp_create_handler_registration("dlgNIUSSD", smscStatsHandler, mapStatDlgNIUSSDOid, OID_LENGTH(mapStatDlgNIUSSDOid), HANDLER_CAN_RONLY);
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

using smsc::snmp::SnmpAgent;
//using smsc::snmp::CLEAR;
//using smsc::snmp::INFO;

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
    uint64_t perf[smsc::system::performance::performanceCounters];
    ((smsc::system::Smsc*)smscptr)->getPerfData(perf);

    int mapDlgStat[6]={0,};
#ifdef USE_MAP
    MapDialogContainer::getInstance()->getDlgStats(mapDlgStat);
#endif

    const int perfBase=6;

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
          val.high = perf[perfBase+0] >> 32;
          val.low  = perf[perfBase+0] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
         smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitOK req");
        }
        else if (snmp_oid_compare(sumbitErrOid,OID_LENGTH(sumbitErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[perfBase+1] >> 32;
          val.low  = perf[perfBase+1] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "submitErr req");
        }
        else if (snmp_oid_compare(deliverOkOid,OID_LENGTH(deliverOkOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high = perf[perfBase+2] >> 32;
          val.low  = perf[perfBase+2] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverOK req");
        }
        else if (snmp_oid_compare(deliverErrOid,OID_LENGTH(deliverErrOid),
                             reginfo->rootoid, reginfo->rootoid_len) == 0)
        {
          val.high =  perf[perfBase+4] >> 32;
          val.low  = perf[perfBase+4] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverERR req");
        }
        else if (snmp_oid_compare(rescheduledOid,OID_LENGTH(rescheduledOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          val.high = perf[perfBase+5] >> 32;
          val.low  = perf[perfBase+5] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
        smsc_log_debug(((smsc::logger::Logger*)agentlog), "rescheduled req");
        }
        else if (snmp_oid_compare(deliverTmpOid,OID_LENGTH(deliverTmpOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          val.high = perf[perfBase+3] >> 32;
          val.low  = perf[perfBase+3] & 0xffffffff;
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64, (u_char *) &val, sizeof(val));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "deliverTEMP req");
        }
        else if (snmp_oid_compare(mapStatDlgInSRIOid,OID_LENGTH(mapStatDlgInSRIOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[0];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgInSRI req");
        }
        else if (snmp_oid_compare(mapStatDlgInOid,OID_LENGTH(mapStatDlgInOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[1];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgIn req");
        }
        else if (snmp_oid_compare(mapStatDlgOutSRIOid,OID_LENGTH(mapStatDlgOutSRIOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[2];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgOutSRI req");
        }
        else if (snmp_oid_compare(mapStatDlgOutOid,OID_LENGTH(mapStatDlgOutOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[3];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgOut req");
        }
        else if (snmp_oid_compare(mapStatDlgUSSDOid,OID_LENGTH(mapStatDlgUSSDOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[4];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgUSSD req");
        }
        else if (snmp_oid_compare(mapStatDlgNIUSSDOid,OID_LENGTH(mapStatDlgNIUSSDOid),
                             reginfo->rootoid, reginfo->rootoid_len) ==0)
        {
          uint32_t val32=mapDlgStat[5];
          snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &val32, sizeof(val32));
          smsc_log_debug(((smsc::logger::Logger*)agentlog), "mapDlgNIUSSD req");
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
