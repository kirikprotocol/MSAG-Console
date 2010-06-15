#ident "$Id$"
#ifndef __SMSC_SNMP_SNMPAGENT_HPP__
#define __SMSC_SNMP_SNMPAGENT_HPP__

#include <stdio.h>

#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace smsc{
class Smsc;
namespace snmp{

using smsc::core::threads::ThreadedTask;

class SnmpAgent: public ThreadedTask{
public:
  enum smscStatus {UNKN,INIT,OPER,SHUT,TERM};
  enum alertSeverity {NORMAL = 1, WARNING = 2, MINOR =3, MAJOR = 4, CRITICAL = 5};
  enum alertStatus {NEW,CLEAR,INFO};
  SnmpAgent(smsc::Smsc* smsc);
  virtual ~SnmpAgent();
  virtual const char *taskName();
  virtual int Execute();
  void statusChange(smscStatus status);
  void trap(const char * const message);
  static void trap(const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
  static void trap(alertStatus status, const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
  void init();
  static smsc::logger::Logger* log;
protected:
  smsc::Smsc* smsc;
};

}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SNMPAGENT_HPP__
