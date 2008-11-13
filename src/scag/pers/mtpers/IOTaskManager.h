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
  void checkAllTasks();

private:
  SortedTask headTask_;
  SortedTask tailTask_;
  SortedTask **sortedTasks_;
  uint16_t maxThreads_;
};

class IOTaskManager {
public:
  IOTaskManager(uint16_t maxThreads, uint32_t maxSock, uint16_t timeout, const char *logName);
  virtual ~IOTaskManager() {};

  virtual bool process(ConnectionContext* cx) = 0;
  virtual void shutdown();
  void removeContext(IOTask* t, uint16_t contextsNumber = 1);
  bool canStop();

private:
  virtual IOTask* newTask() = 0;

protected:
  void init();

protected:
  TasksSorter taskSorter_;
  Mutex tasksMutex_;
  uint32_t maxSockets_;    
  uint16_t connectionTimeout_;
  Logger *logger;

private:
  ThreadPool pool_;
  uint16_t maxThreads_;
};

}//mtpers
}//scag

#endif

