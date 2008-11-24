#ifndef __SCAG_MTPERS_WRITERTASKMANAGER_H__
#define __SCAG_MTPERS_WRITERTASKMANAGER_H__

#include "scag/util/RelockMutexGuard.h"
#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag { namespace mtpers {

using scag::util::RelockMutexGuard;

class WriterTaskManager : public IOTaskManager {
public:
  WriterTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout) 
                   : IOTaskManager(maxThreads, maxSock, timeout, "writerman") {} 

  IOTask* newTask() {
    return new MTPersWriter(*this, connectionTimeout_);
  }

  bool process(ConnectionContext* cx) {
    RelockMutexGuard g(tasksMutex_);
    if (isStopped_) {
      return false;
    }
    IOTask *t = (IOTask*)taskSorter_.getFirst();
    if (t->getSocketsCount() < maxSockets_) {
      t->registerContext(cx);
      taskSorter_.reorderTask(t);
      smsc_log_debug(logger_, "%p:%d choosen for context %p", t, t->getSocketsCount(), cx);
      return true;
    } else {
      g.Unlock();
      smsc_log_warn(logger_, "Can't process %p context. Server busy. Max sockets=%d, current sockets=%d", cx,  maxSockets_, t->getSocketsCount());
      return false;
    }
  }
};

}//mtpers
}//scag

#endif
  
 
