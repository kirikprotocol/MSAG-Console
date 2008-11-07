#ifndef __SCAG_MTPERS_WRITERTASKMANAGER_H__
#define __SCAG_MTPERS_WRITERTASKMANAGER_H__

#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag { namespace mtpers {

class WriterTaskManager : public IOTaskManager {
public:
  WriterTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout) 
                   : IOTaskManager(maxThreads, maxSock, timeout, "writerman") { init(); }; 

  IOTask* newTask() {
    return new MTPersWriter(*this, connectionTimeout_);
  }

  bool process(ConnectionContext* cx) {
    MutexGuard g(tasksMutex_);
    IOTask *t = (IOTask*)taskSorter_.getFirst();
    if (t->getSocketsCount() < maxSockets_) {
      smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketsCount(), cx);
      t->registerContext(cx);
      taskSorter_.reorderTask(t);
      return true;
    } else {
      smsc_log_warn(logger, "Can't process %p context. Server busy. Max sockets=%d, current sockets=%d", cx,  maxSockets_, t->getSocketsCount());
      return false;
    }
  }
};

}//mtpers
}//scag

#endif
  
 
