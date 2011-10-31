#include <limits.h>
#include "IOTaskManager.h"

namespace scag2 {
namespace pvss  {

using smsc::logger::Logger;
using namespace smsc::core::synchronization;

/*
TasksSorter::TasksSorter() : maxThreads_(0), headTask_(0), tailTask_(UINT_MAX) {
}

void TasksSorter::init(uint16_t maxThreads) {
  maxThreads_ = maxThreads;
  sortedTasks_ = new SortedTask *[maxThreads_ + 2];
  sortedTasks_[0] = &headTask_;               
  sortedTasks_[maxThreads + 1] = &tailTask_;  
}

void TasksSorter::checkAllTasks() {
  Logger* logger = Logger::getInstance("sorter");
  for (int i = 1; i <= maxThreads_; ++i) {
    smsc_log_debug(logger, "task:%p i:%d index:%d sockets:%d", sortedTasks_[i], i, sortedTasks_[i]->index_, sortedTasks_[i]->itemsCount_);
  }
}

TasksSorter::~TasksSorter() {
  if (sortedTasks_) {
    delete sortedTasks_;
  }
}

SortedTask* TasksSorter::getTask(uint16_t index) {
  if (index > maxThreads_) {
    return NULL;
  }
  return sortedTasks_[index];
}


SortedTask* TasksSorter::getFirst() {
  return sortedTasks_[1];
}

void TasksSorter::reorderTask(SortedTask* task) {
  uint16_t index = task->index_;
  uint32_t count = task->itemsCount_;

  if (count > sortedTasks_[index+1]->itemsCount_) {
    do {        
      assignTask(index, sortedTasks_[index+1]);
    } while (count > sortedTasks_[++index+1]->itemsCount_);
    assignTask(index, task);
  }
  else if (count < sortedTasks_[index-1]->itemsCount_) {
    do {
      assignTask(index, sortedTasks_[index-1]);
    } while (count < sortedTasks_[--index-1]->itemsCount_);
    assignTask(index, task);
  }
}

void TasksSorter::assignTask(uint16_t index, SortedTask *task) {
  sortedTasks_[index] = task;
  task->index_ = index;
}
 */


// ----------------------------------------------------------


IOTaskManager::IOTaskManager( const SyncConfig& cfg,
                              const char *logName ) :
log_(Logger::getInstance(logName)),
isStopped_(true),
connectionTimeout_(cfg.getConnectTimeout()),
ioTimeout_(cfg.getIoTimeout()),
maxThreads_(cfg.getIoTasksCount()),
maxSockets_((cfg.getMaxClientsCount() + maxThreads_ - 1) / maxThreads_)
{
}


void IOTaskManager::init() 
{
    MutexGuard mg(tasksMutex_);
    pool_.setMaxThreads(maxThreads_);
    for (uint16_t i = 1; i <= maxThreads_; ++i) {
        IOTask *t = newTask();
        tasks_.Push(t);
        pool_.startTask(t);
    }
    isStopped_ = false;
}

/*
void IOTaskManager::removeContext(IOTask* t, uint16_t contextsNumber) {
  MutexGuard g(tasksMutex_);
  t->itemsCount_ -= contextsNumber;
  taskSorter_.reorderTask(t);
}
 */

/*
bool IOTaskManager::canStop() 
{
    MutexGuard g(tasksMutex_);
    const int tc = tasks_.Count();
    for ( int i = 0; i < tc; ++i ) {
        if ( tasks_[i]->getSocketCount() ) return false;
    }
    return true;
}
 */


void IOTaskManager::shutdown() 
{
    {
        MutexGuard g(tasksMutex_);
        if (isStopped_) return;
        isStopped_ = true;
    }
    pool_.shutdown();
}


IOTaskManager::~IOTaskManager() 
{
    shutdown();
}

}//pvss
}//scag2
