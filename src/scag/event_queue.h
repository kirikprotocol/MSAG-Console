/*
  $Id$
*/


#if !defined (__Cxx_SCAG_Header__EventQueue_h__)
#define __Cxx_SCAG_Header__EventQueue_h__

#include "core/buffers/PriorityQueue.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "smeman/smsccmd.h"
#include "scag/state_checker.hpp"
//#include <stdexcept>
#include <inttypes.h>
//#include <stdint.h>
#include <string.h>
#include <list>

#define DISABLE_LIST_DUMP

namespace scag {

using namespace smsc::smeman;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

class EventQueue
{
  public:
  uint64_t counter;

  // запись таблицы блокировок

  Event event;
  Mutex mutex;


  typedef PriorityQueue<CommandType,CyclicQueue<CommandType>,0,31> LockerQueue;
  LockerQueue queue;

public:
#define __synchronized__ MutexGuard mguard(mutex);

  EventQueue() : counter(0)
  {
  }

  ~EventQueue() {}

  uint64_t getCounter()
  {
  __synchronized__
    return counter;
  }

  void getStats(int& qcnt)
  {
    __synchronized__
    qcnt=queue.Count();
  }

  void enqueue(const CommandType& command)
  {
  __synchronized__
    __trace2__("enqueue:cmd=%d",command->get_commandId());
    queue.Push(command,command->get_priority());
    event.Signal();
  }

  void selectAndDequeue(CommandType& result,volatile bool* quitting)
  {
    for(;;)
    {
      {
        __synchronized__
        if(queue.Count()>0)
        {
          queue.Pop(result);
          counter++;
          return;
        }
      }
      event.Wait(2000);
      if(*quitting)return;
    }
  }

#undef __synchronized__
};

#undef DISABLE_LIST_DUMP


} // namespace system


#endif
