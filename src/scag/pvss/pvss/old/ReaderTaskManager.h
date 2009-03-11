#ifndef _SCAG_PVSS_SERVER_READERTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_READERTASKMANAGER_H_

#include "scag/util/RelockMutexGuard.h"
#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag2 {
namespace pvss  {

using scag::util::RelockMutexGuard;

class ReaderTaskManager : public IOTaskManager {
public:
  ReaderTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout, bool perfCounterOn = false) 
                   : IOTaskManager(maxThreads, maxSock, timeout, "readerman"), perfCounterOn_(perfCounterOn) {}

  IOTask* newTask() {
    MTPersReader* reader = new MTPersReader(*this, connectionTimeout_);
    if (perfCounterOn_) {
      readers_.push_back(reader);
    }
    return reader;
  }

  bool process(ConnectionContext* cx) {
    RelockMutexGuard g(tasksMutex_);
    if (isStopped_) {
      return false;
    }
    IOTask *t = (IOTask*)taskSorter_.getFirst();
    if (t->getSocketsCount() < maxSockets_) {
      cx->getSocket()->Write("OK", 2);
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
/*
  bool processPacket(RequestPacket* packet) {
    return storageManager_.process(packet);
  }
*/
  void shutdown() {
    readers_.clear();
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
  //StorageManager& storageManager_;
  std::vector<MTPersReader*> readers_;
  bool perfCounterOn_;
};

}//pvss
}//scag2

#endif
  
 
