#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "store/StoreManager.h"
#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Event.hpp"

namespace smsc{
namespace system{

using smsc::store::MessageStore;
using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::synchronization::Event;

class Scheduler:public smsc::core::threads::ThreadedTask{
public:
  Scheduler(EventQueue& eq,MessageStore* st):
    queue(eq),store(st),rescheduleLimit(10){}
  int Execute()
  {
    time_t t,r;
    smsc::store::IdIterator *it=NULL;
    Array<SMSId> ids;
    SMSId id;
    Event e;
    //int cnt;
    while(!isStopping)
    {
      t=time(NULL);
      __trace__("scheduler started");
      try{
        it=store->getReadyForRetry(t,true);
        if(it)
        {
          try{
            while(it->getNextId(id))
            {
              ids.Push(id);
              if(ids.Count()>=rescheduleLimit*2)break;
            }
          }catch(...)
          {
            __trace__("Scheduler: Exception in getReadyForRetry");
          }
          delete it;
        }
        it=store->getReadyForRetry(t);
        if(it)
        {
          try{
            while(it->getNextId(id))
            {
              ids.Push(id);
              if(ids.Count()>=rescheduleLimit*2)break;
            }
          }catch(...)
          {
            __trace__("Scheduler: Exception in getReadyForRetry");
          }
          delete it;
        }
        if(ids.Count())
        {
          __trace2__("Scheduler: %d messages for rescheduling",ids.Count());
          try{
            for(int i=0;i<ids.Count();i++)
            {
              SmscCommand cmd=SmscCommand::makeForward();
              queue.enqueue(ids[i],cmd);
              //thr_yield();
              e.Wait(1000/rescheduleLimit);
              if(isStopping)break;
            }
          }catch(...)
          {
            __trace__("Scheduler: Exception queue.enqueue");
          }
          ids.Clean();
        }
      }catch(...)
      {
        __trace__("Scheduler: Exception in getReadyForRetry");
      }
      if(isStopping)break;
      it=NULL;
      __trace__("scheduler finished, sleeping");
      sleep(1);
      r=store->getNextRetryTime();
      if(r==0)
        event.Wait();
      else
        if(r>t)event.Wait((r-t)*1000);
    }
    return 0;
  }
  const char* taskName(){return "scheduler";}

  void notify()
  {
    event.Signal();
  }
  void setRescheduleLimit(int rl)
  {
    rescheduleLimit=rl;
  }
protected:
  EventQueue &queue;
  MessageStore* store;
  Event event;
  int rescheduleLimit;
};

};//system
};//smsc


#endif
