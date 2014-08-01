#ident "$Id$"
#ifndef __MSAG_SNMP_SNMPAGENT_HPP__
#define __MSAG_SNMP_SNMPAGENT_HPP__

#include <stdio.h>

#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace snmp {

extern "C" void dummyAlert(unsigned,void*);

//using smsc::core::threads::ThreadedTask;

class SnmpAgent //: public ThreadedTask
{
public:
  enum msagStatus {UNKN,INIT,OPER,SHUT,TERM};
//  enum alertSeverity {NORMAL = 1, WARNING = 2, MINOR =3, MAJOR = 4, CRITICAL = 5};
//  enum alertStatus {NEW,CLEAR,INFO};
  SnmpAgent(unsigned node_number);
  virtual ~SnmpAgent();
//  virtual const char *taskName();
//  virtual int Execute();
  void statusChange(msagStatus status);
  void init();
  void shutdown();
/*
  void trap(const char * const message);
  static void trap(const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
  static void trap(alertStatus status, const char * const alarmId, const char * const alarmObjCategory, alertSeverity severity, const char * const text);
*/
  static smsc::logger::Logger* log;
protected:
  unsigned node;
};

}}

#endif
