#ifndef __SCAG_MTPERS_READERTASKMANAGER_H__
#define __SCAG_MTPERS_READERTASKMANAGER_H__

#include "IOTaskManager.h"
#include "IOTask.h"
#include "StorageManager.h"

namespace scag { namespace mtpers {

class ReaderTaskManager : public IOTaskManager {
public:
  ReaderTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout, StorageManager& storageManager) 
                   : IOTaskManager(maxThreads, maxSock, timeout, "readerman"), storageManager_(storageManager) { init(); }; 

  IOTask* newTask() {
    return new MTPersReader(*this, connectionTimeout_);
  }
  //~ReaderTaskManager() {
    //storageManager_.shutdown();
  //}

  bool process(ConnectionContext* cx) {
    MutexGuard g(tasksMutex_);
    IOTask *t = (IOTask*)taskSorter_.getFirst();
    if (t->getSocketsCount() < maxSockets_) {
      cx->getSocket()->Write("OK", 2);

      smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketsCount(), cx);
      t->registerContext(cx);
      taskSorter_.reorderTask(t);
      return true;
    } else {
      smsc_log_warn(logger, "Can't process %p context. Server busy. Max sockets=%d, current sockets=%d", cx,  maxSockets_, t->getSocketsCount());
      return false;
    }
  }
  bool processPacket(PersPacket* packet) {
    return storageManager_.process(packet);
  }

  void shutdown() {
    storageManager_.shutdown();
    IOTaskManager::shutdown();
  }

private:
  StorageManager& storageManager_;
};

}//mtpers
}//scag

#endif
  
 
