#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "core/threads/ThreadedTask.hpp"
#include "store/StoreManager.h"
#include "system/event_queue.h"

namespace smsc{
namespace system{

class smsc::system::Smsc;

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::system::Smsc *app):

               eq(q),
               store(st),
               smsc(app){}
  virtual ~StateMachine(){}

  int Execute();

  const char *taskName(){return "StateMachine";}

protected:
  EventQueue& eq;
  smsc::store::MessageStore* store;
  smsc::system::Smsc *smsc;

  StateType submit(Tuple& t);
  StateType forward(Tuple& t);
  StateType deliveryResp(Tuple& t);

};

};//system
};//smsc

#endif
