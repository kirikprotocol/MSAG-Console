#ifndef _SCAG_PVSS_SERVER_IOTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_IOTASKMANAGER_H_

#include <algorithm>
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

#include "SyncConfig.h"
#include "ConnectionContext.h"
#include "IOTask.h"
#include "util/PtrLess.h"

namespace scag2 {
namespace pvss  {

// using smsc::core::threads::ThreadPool;
// using smsc::core::synchronization::MutexGuard;
// using smsc::core::synchronization::Mutex;
// using smsc::logger::Logger;

/*
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
 */


/// base class for manager of homogeneous iotasks
class IOTaskManager 
{
public:
    IOTaskManager( const SyncConfig& cfg, const char *logName );
    virtual ~IOTaskManager();

    void init();
    void shutdown();

    bool registerConnection( ConnectionContext* cx )
    {
        unsigned sc;
        {
            smsc::core::synchronization::MutexGuard mg(tasksMutex_);
            if (isStopped_) return false;
            if (!tasks_.Count()) return false;
            IOTask* t = *std::min_element( &tasks_[0],
                                           &tasks_[0] + tasks_.Count(),
                                           smsc::util::PtrLess() );
            sc = t->getSocketsCount();
            if ( sc < maxSockets_ ) {
                postRegister(cx);
                t->registerContext(cx);
                smsc_log_debug(log_,"%p:%d choosen for context %p",
                               t, sc, cx );
                return true;
            }
        }
        smsc_log_debug(log_,"Can't process %p context. Server busy. Max sockets=%d, current sockets=%d",
                       cx,  maxSockets_, sc);
        return false;
    }

protected:
    virtual void postRegister(ConnectionContext* cx) = 0;
    virtual IOTask* newTask() = 0;

protected:
    smsc::logger::Logger*  log_;
    bool                   isStopped_;
    const uint16_t connectionTimeout_;
    const uint16_t ioTimeout_;

private:
    const uint32_t maxThreads_;        // total
    const uint32_t maxSockets_;        // per thread

    // TasksSorter taskSorter_;
    smsc::core::synchronization::Mutex    tasksMutex_;
    smsc::core::buffers::Array< IOTask* > tasks_;
    smsc::core::threads::ThreadPool       pool_;
};

}//pvss
}//scag2

#endif

