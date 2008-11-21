#ifndef __SCAG_MTPERS_READERTASKMANAGER_H__
#define __SCAG_MTPERS_READERTASKMANAGER_H__

#include "scag/util/RelockMutexGuard.h"
#include "IOTaskManager.h"
#include "IOTask.h"
#include "StorageManager.h"

namespace scag { namespace mtpers {

using scag::util::RelockMutexGuard;

class ReaderTaskManager : public IOTaskManager {
public:
  ReaderTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout, StorageManager& storageManager, bool perfCounterOn = false) 
                   : IOTaskManager(maxThreads, maxSock, timeout, "readerman"), storageManager_(storageManager), perfCounterOn_(perfCounterOn)
  {
     init();
  }; 

  IOTask* newTask() {
    MTPersReader* reader = new MTPersReader(*this, connectionTimeout_);
    if (perfCounterOn_) {
      readers_.push_back(reader);
    }
    return reader;
  }

  bool process(ConnectionContext* cx) {
    RelockMutexGuard g(tasksMutex_);
    IOTask *t = (IOTask*)taskSorter_.getFirst();
    if (t->getSocketsCount() < maxSockets_) {
      cx->getSocket()->Write("OK", 2);
      t->registerContext(cx);
      taskSorter_.reorderTask(t);
      smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketsCount(), cx);
      return true;
    } else {
      g.Unlock();
      smsc_log_warn(logger, "Can't process %p context. Server busy. Max sockets=%d, current sockets=%d", cx,  maxSockets_, t->getSocketsCount());
      return false;
    }
  }

  bool processPacket(PersPacket* packet) {
    return storageManager_.process(packet);
  }

  void shutdown() {
    readers_.clear();
    storageManager_.shutdown();
    IOTaskManager::shutdown();
  }

  Performance getPerformance() {
    Performance perf;
    if (!perfCounterOn_) {
      return perf;
    }
    for (std::vector<MTPersReader*>::iterator i = readers_.begin(); i != readers_.end(); ++i) {
      perf.inc((*i)->getPerformance());
    }
    return perf;
  }
private:
  StorageManager& storageManager_;
  std::vector<MTPersReader*> readers_;
  bool perfCounterOn_;
};

}//mtpers
}//scag

#endif
  
 
