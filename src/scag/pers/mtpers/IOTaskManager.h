#ifndef __SCAG_PERS_PVGW_IOTASKMANAGER_H__
#define __SCAG_PERS_PVGW_IOTASKMANAGER_H__

#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

#include "ConnectionContext.h"
#include "IOTask.h"
#include "StorageManager.h"

namespace scag { namespace mtpers {

using smsc::core::threads::ThreadPool;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;

class TasksSorter {
public:
  TasksSorter();
  virtual ~TasksSorter();
  void init(uint16_t maxThreads);
  SortedTask* getFirst();
  void reorderTask(SortedTask* t);
  void assignTask(uint16_t index, SortedTask *task);
  SortedTask* getTask(uint16_t index);

private:
  SortedTask headTask_;
  SortedTask tailTask_;
  SortedTask **sortedTasks_;
  uint16_t maxThreads_;
};

class IOTaskManager {
public:
  IOTaskManager(StorageManager& storageManager);
  virtual ~IOTaskManager() {};

  bool process(ConnectionContext* cx);
  bool storageProcess(ConnectionContext* cx);
  void shutdown();
  void removeContext(IOTask* t, uint16_t contextsNumber = 1);
  void init(uint16_t maxThreads, uint32_t maxSock, const char *logName);
  bool canStop();
  bool serverReady();

private:
  IOTask* newTask();
  inline void giveContext(IOTask *t, ConnectionContext* cx);

private:
  ThreadPool pool_;
  Mutex tasksMutex_;
  Logger *logger;
  uint32_t maxSockets_;    
  uint16_t maxThreads_;
  uint16_t connectionTimeout_;
  TasksSorter taskSorter_;
  StorageManager& storageManager_;
};

}//mtpers
}//scag

#endif

