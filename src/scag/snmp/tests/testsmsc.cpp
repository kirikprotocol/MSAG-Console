#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "logger/Logger.h"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace snmp {

class SnmpAgent {
public:
  enum smscStatus {UNKN,INIT,OPER,SHUT,TERM};
  enum alertSeverity {NORMAL = 1, WARNING = 2, MINOR =3, MAJOR = 4, CRITICAL = 5};
  enum alertStatus {NEW,CLEAR,INFO};

    SnmpAgent();
    ~SnmpAgent() {}
    // void statusChange(smscStatus status);
    // void trap(const char * const message);
    // static void trap(const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
    // static void trap(alertStatus status, const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
    void init();
    static smsc::logger::Logger* log;
};

struct TrapRecord {
    TrapRecord();

    static const char* headerText();

    enum RecordType{
      rtTrap,rtNotification,rtStatusChange
    };
    RecordType recordType;
    time_t submitTime;
    int status;
    smsc::core::buffers::FixedLengthString<32> alarmId;
    smsc::core::buffers::FixedLengthString<32> alarmObjCategory;
    SnmpAgent::alertSeverity                   severity;
    std::string                                text;
};

}
}

using smsc::snmp::SnmpAgent;
using smsc::snmp::TrapRecord;

extern "C" void dummyAlert(unsigned,void*)
{
}

namespace smsc {
namespace snmp {

const char    _TrapRecordHeader_TEXT[] =
    "SUBMIT_TIME,ALARM_ID,ALARMCATEGORY,SEVERITY,TEXT\n";

const char* TrapRecord::headerText() { return _TrapRecordHeader_TEXT;}

TrapRecord::TrapRecord() {}


smsc::logger::Logger* SnmpAgent::log = 0;

SnmpAgent::SnmpAgent()
{
    log = smsc::logger::Logger::getInstance("sms.snmp");
    init();
}


void SnmpAgent::init()
{
    // gettimeofday(&agentStartTime, NULL);
    // agentStartTime.tv_sec--;
    // agentStartTime.tv_usec += 1000000L;
    // snmp_disable_stderrlog();  // print log errors to stderr
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_ROLE, 1);  // we are a subagent
    init_agent("smscd");  // initialize the agent library
    // init_smsc();          // initialize mib code here
    init_snmp("smsc");    //read .conf files
    // smestattable::SmeStatTableSubagent::Register();
    // smeerrtable::InitSmeErrTable();
    smsc_log_debug(log,"snmp agent inited");
}


}
}



namespace {

oid alertIdOid[] =            { 1, 3, 6, 1, 4, 1, 26757, 1, 8, 0 };
oid alertMessageOid[] =       { 1, 3, 6, 1, 4, 1, 26757, 1, 5, 0 };
oid alertObjCategoryOid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 9, 0 };
oid alertOid[] =              { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 2 };
oid alertSeverityOid[] =      { 1, 3, 6, 1, 4, 1, 26757, 1, 7, 0 };
oid smscAlertFFMROid[] =      { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 3 };
oid smscClearAlertFFMROid[] = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 5 };
oid smscNewAlertFFMROid[] =   { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 4 };
oid statusNotificationOid[] = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 1 };
oid status_oid[] =            { 1, 3, 6, 1, 4, 1, 26757, 1, 2, 0 };

    void
    ProcessTrap( TrapRecord* vars)
    {
      if(vars->recordType==TrapRecord::rtTrap)
      {
        // if(fileLog) fileLog->log(*vars);
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
        // if(fileLog) fileLog->log(*vars);
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
}


int main()
{
    smsc::logger::Logger::Init();
    
    SnmpAgent agent;

    struct timeval t;t.tv_sec=0,t.tv_usec=500000;
    snmp_alarm_register_hr(t, SA_REPEAT, dummyAlert, 0);

    TrapRecord rec;
    rec.recordType = TrapRecord::rtTrap;
    rec.submitTime = time(0);
    rec.status = 1;
    rec.alarmId = "smsc";
    rec.alarmObjCategory = "connect";
    rec.severity = SnmpAgent::MAJOR;
    rec.text = "hello,world";

    agent_check_and_process(1);
    ProcessTrap( &rec );

    sleep(3);
    return 0;
}
