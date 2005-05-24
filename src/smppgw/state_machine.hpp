#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "smppgw/smsc.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "smppgw/event_queue.h"
#include "util/templates/Formatters.h"
#include <string>
#include "profiler/profiler.hpp"
#include "core/buffers/Hash.hpp"
#include <list>
#include <exception>
#include "TrafficWriter.h"

namespace smsc{
namespace smppgw{

class smsc::smppgw::Smsc;
using smsc::profiler::Profile;

using namespace smsc::util::templates;

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::smppgw::Smsc *app);
  virtual ~StateMachine()
  {
  }

  int Execute();

  const char *taskName(){return "StateMachine";}

  static smsc::db::DataSource* dataSource;

  static void initHourlyCounters(const char* location);

  static Mutex trafficMutex;

protected:

  EventQueue& eq;
  smsc::smppgw::Smsc *smsc;

  smsc::logger::Logger* smsLog;


  void submit(SmscCommand& cmd);
  void submitResp(SmscCommand& cmd);
  void delivery(SmscCommand& cmd);
  void deliveryResp(SmscCommand& cmd);
  void alert(SmscCommand& cmd);
  void replace(SmscCommand& cmd);
  void replaceResp(SmscCommand& cmd);
  void query(SmscCommand& cmd);
  void queryResp(SmscCommand& cmd);
  void cancel(SmscCommand& cmd);
  void cancelResp(SmscCommand& cmd);

  void KillExpiredTrans();

  static void updateTraffic(const char* routId);

};

}//smppgw
}//smsc

#endif
