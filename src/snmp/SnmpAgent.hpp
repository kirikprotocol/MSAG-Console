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
  SnmpAgent(smsc::system::Smsc* smsc);
  virtual ~SnmpAgent();
  virtual const char *taskName();
  virtual int Execute();
  void statusChange(smscStatus status);
  void trap(std::string &message);
  void init();
protected:
  smsc::logger::Logger* log;
  smsc::system::Smsc* smsc;
};

}//snmp name space
}//smsc name space

#endif // __SMSC_SNMP_SNMPAGENT_HPP__
