#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "smppgw/smsc.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "system/event_queue.h"
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
  StateMachine(smsc::system::EventQueue& q,
               smsc::smppgw::Smsc *app);
  virtual ~StateMachine()
  {
  }

  int Execute();

  const char *taskName(){return "StateMachine";}

  time_t maxValidTime;
  Address scAddress;

protected:

  smsc::system::EventQueue& eq;
  smsc::smppgw::Smsc *smsc;

  smsc::logger::Logger* smsLog;

  std::string serviceType;
  int protocolId;
  std::string smscSmeId;

  smsc::system::StateType submit(smsc::system::Tuple& t);
  smsc::system::StateType submitResp(smsc::system::Tuple& t);
  smsc::system::StateType delivery(smsc::system::Tuple& t);
  smsc::system::StateType deliveryResp(smsc::system::Tuple& t);
  smsc::system::StateType alert(smsc::system::Tuple& t);
  smsc::system::StateType replace(smsc::system::Tuple& t);
  smsc::system::StateType query(smsc::system::Tuple& t);
  smsc::system::StateType cancel(smsc::system::Tuple& t);


};

}//smppgw
}//smsc

#endif
