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
#include "util/regexp/RegExp.hpp"
#include <exception>

namespace smsc{
namespace smppgw{

class smsc::smppgw::Smsc;
using smsc::profiler::Profile;

using namespace smsc::util::templates;

class RegExpCompilationException:public std::exception{
public:
  const char* what()const throw()
  {
    return "failed to compile directive processing regexp in state machine";
  }
};

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::smppgw::Smsc *app);
  virtual ~StateMachine()
  {
  }

  int Execute();

  const char *taskName(){return "StateMachine";}

  time_t maxValidTime;
  Address scAddress;

  static smsc::db::DataSource* dataSource;

protected:

  EventQueue& eq;
  smsc::smppgw::Smsc *smsc;

  smsc::logger::Logger* smsLog;

  std::string serviceType;
  int protocolId;
  std::string smscSmeId;


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

};

}//smppgw
}//smsc

#endif
