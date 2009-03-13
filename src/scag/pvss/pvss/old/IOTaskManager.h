#ifndef _SCAG_PVSS_SERVER_IOTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_IOTASKMANAGER_H_

#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

#include "ConnectionContext.h"
#include "IOTask.h"

namespace scag2 {
namespace pvss  {

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
  virtual ~IOTaskManager();

  virtual bool process(ConnectionContext* cx) = 0;
  virtual void shutdown();
  void removeContext(IOTask* t, uint16_t contextsNumber = 1);
  bool canStop();
  void init();

private:
  virtual IOTask* newTask() = 0;

protected:
  bool isStopped_;
  uint32_t maxSockets_;    
  uint16_t connectionTimeout_;
  Logger *logger_;
  TasksSorter taskSorter_;
  Mutex tasksMutex_;

private:
  ThreadPool pool_;
  uint16_t maxThreads_;
};

}//pvss
}//scag2

#endif

