#ifndef _SCAG_PVSS_SERVER_WRITERTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_WRITERTASKMANAGER_H_

#include "scag/util/RelockMutexGuard.h"
#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag2 {
namespace pvss  {

using scag::util::RelockMutexGuard;

class WriterTaskManager : public IOTaskManager {
public:
  WriterTaskManager(const SyncConfig& cfg) : IOTaskManager(cfg, "writerman") {} 

  IOTask* newTask() {
    return new MTPersWriter(*this, connectionTimeout_, ioTimeout_);
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

}//pvss
}//scag2

#endif
  
 
