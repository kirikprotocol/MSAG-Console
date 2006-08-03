#ifndef __SMSC_SNMP_SNMPAGENT_HPP__
#define __SMSC_SNMP_SNMPAGENT_HPP__

#include <stdio.h>
#include "core/threads/ThreadedTask.hpp"
namespace smsc{namespace system{class Smsc;}}
namespace smsc{
namespace snmp{

using smsc::core::threads::ThreadedTask;

class SnmpAgent: public ThreadedTask{
public:
  enum smscStatus {UNKN,INIT,OPER,SHUT,TERM};
  enum alertSeverity {NORMAL = 1, WARNING = 2, MINOR =3, MAJOR = 4, CRITICAL = 5};
  SnmpAgent(smsc::system::Smsc* smsc);
  virtual ~SnmpAgent();
  virtual const char *taskName();
  virtual int Execute();
  void statusChange(smscStatus status);
  void trap(const char * const message);
  static void trap(const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
  void init();
protected:
  smsc::logger::Logger* log;
  smsc::system::Smsc* smsc;
};

}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SNMPAGENT_HPP__
