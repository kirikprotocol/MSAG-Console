#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "core/threads/ThreadedTask.hpp"
#include "store/StoreManager.h"
#include "system/event_queue.h"
#include "util/templates/Formatters.h"
#include <string>

namespace smsc{
namespace system{

class smsc::system::Smsc;

using namespace smsc::util::templates;

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::system::Smsc *app):
               eq(q),
               store(st),
               smsc(app),
               ofDelivered(0),
               ofFailed(0)
               {}
  virtual ~StateMachine()
  {
    if(ofDelivered)delete ofDelivered;
    if(ofFailed)delete ofFailed;
  }

  int Execute();

  void initFormatters(const char* deliver,const char* failed)
  {
    ofDelivered=new OutputFormatter(deliver);
    ofFailed=new OutputFormatter(failed);
  }

  void formatDeliver(const char* addr,time_t date,std::string& out);
  void formatFailed(const char* addr,const char* err,std::string& out);

  void sendFailureReport(SMS& sms,MsgIdType msgId,const char* reason);

  const char *taskName(){return "StateMachine";}

  time_t maxValidTime;
  Address scAddress;

protected:
  EventQueue& eq;
  smsc::store::MessageStore* store;
  smsc::system::Smsc *smsc;

  OutputFormatter *ofDelivered;
  OutputFormatter *ofFailed;

  StateType submit(Tuple& t);
  StateType forward(Tuple& t);
  StateType deliveryResp(Tuple& t);
  StateType alert(Tuple& t);

};

};//system
};//smsc

#endif
