static char const ident[] = "$Id$";
#include "snmp/sctp/SnmpTrapSender.hpp"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <inttypes.h>
#include "snmp/TrapRecordLog.hpp"
#include "util/config/Manager.h"

namespace smsc{
namespace snmp{
namespace sctp{

using smsc::snmp::TrapRecordLog;
using smsc::inman::filestore::InFileStorageRoller;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;
using smsc::logger::Logger;

static int keep_running;
static char cfgname[] = "sctpmon";
static TrapRecordLog*       fileLog = 0;
static InFileStorageRoller* fileLogRoller = 0;
static const char *         fileLogLocation = ".";
static unsigned long        fileLogRollInterval = 60;
static Logger* logger = 0;

void SnmpTrapSender::run()
{
  Start();
}
void SnmpTrapSender::stop()
{
  keep_running = 0;
}

SnmpTrapSender::SnmpTrapSender()
{
  logger = Logger::getInstance("sctp.trap");
  snmp_enable_stderrlog();
  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
  init_agent(cfgname); /* initialize the agent library */
  init_snmp(cfgname); /* example-demon will be used to read example-demon.conf files. */
}

int SnmpTrapSender::Execute(void)
{
  Manager& manager = Manager::getInstance();
  try { fileLogLocation = manager.getString("sctpmon.csvFileDir");
  } catch (ConfigException& exc) { fileLogLocation="."; }
  try { fileLogRollInterval = manager.getInt("sctpmon.csvFileRollInterval");
  } catch (ConfigException& exc) { fileLogRollInterval = 60; }
  fileLog = new TrapRecordLog(fileLogLocation, "sgn.lst", "sgn.csv", 0, logger);
  fileLog->RFSOpen(true);
  fileLogRoller = new InFileStorageRoller(fileLog,fileLogRollInterval);
  if (fileLogRoller) { fileLogRoller->Start(); }

  smsc_log_debug(logger,"TrapSender is running");
  keep_running = 1;
  while(keep_running) {
    agent_check_and_process(1); /* 0 == don't block */
  }
  if (fileLogRoller) {
    fileLogRoller->Stop();
//    fileLogRoller->WaitFor();
  }
  snmp_shutdown(cfgname);
  smsc_log_debug(logger,"TrapSender is down");
  return 0;
}

static oid text_oid[]        = { 1, 3, 6, 1, 4, 1, 26757, 1, 5, 0 };
static oid severity_oid[]    = { 1, 3, 6, 1, 4, 1, 26757, 1, 7, 0 };
static oid alarmid_oid[]     = { 1, 3, 6, 1, 4, 1, 26757, 1, 8, 0 };
static oid object_oid[]      = { 1, 3, 6, 1, 4, 1, 26757, 1, 9, 0 };
static oid alarm_oid[]       = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 3 };
static oid new_alarm_oid[]   = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 4 };
static oid clear_alarm_oid[] = { 1, 3, 6, 1, 4, 1, 26757, 1, 0, 5 };


void SnmpTrapSender::trap(SnmpAgent::alertStatus st,
                      const char * const alarmId,
                      const char * const object,
                      SnmpAgent::alertSeverity svr,
                      const char * const text)
{
    netsnmp_variable_list *notification_vars = NULL;
    snmp_varlist_add_variable(&notification_vars,text_oid,OID_LENGTH(text_oid),ASN_OCTET_STR,(u_char*)text,strlen(text));
    uint8_t s = svr;
    snmp_varlist_add_variable(&notification_vars,severity_oid,OID_LENGTH(severity_oid),ASN_INTEGER,&s,sizeof(uint8_t));
    snmp_varlist_add_variable(&notification_vars,object_oid,OID_LENGTH(object_oid),ASN_OCTET_STR,(u_char*)object,strlen(object));
    snmp_varlist_add_variable(&notification_vars,alarmid_oid,OID_LENGTH(alarmid_oid),ASN_OCTET_STR,(u_char*)alarmId,strlen(alarmId));
    oid* poid = 0; int  oidlen = 0;
    switch (st) {
      case SnmpAgent::INFO : poid = alarm_oid; oidlen = (int)OID_LENGTH(alarm_oid); break;
      case SnmpAgent::NEW  : poid = new_alarm_oid; oidlen = (int)OID_LENGTH(new_alarm_oid); break;
      case SnmpAgent::CLEAR: poid = clear_alarm_oid; oidlen = (int)OID_LENGTH(clear_alarm_oid); break;
    }
    send_enterprise_trap_vars(SNMP_TRAP_ENTERPRISESPECIFIC, 1, poid, oidlen, notification_vars);
    snmp_free_varbind(notification_vars);

  TrapRecord rec;
  rec.submitTime = time(0);
  rec.alarmId = alarmId;
  rec.alarmObjCategory = object;
  rec.severity = svr;
  rec.text = text;
  if(fileLog) fileLog->log(rec);
  smsc_log_debug(logger,"TrapSender sent a trap");
}
}//namespace sctp
}//namespace snmp
}//namespace smsc
