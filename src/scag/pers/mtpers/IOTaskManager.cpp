#include "IOTaskManager.h"

namespace scag { namespace mtpers {

TasksSorter::TasksSorter():maxThreads_(0), headTask_(0), tailTask_(UINT_MAX) {
}

void TasksSorter::init(uint16_t maxThreads) {
  maxThreads_ = maxThreads;
  sortedTasks_ = new SortedTask *[maxThreads_ + 2];
  sortedTasks_[0] = &headTask_;               
  sortedTasks_[maxThreads + 1] = &tailTask_;  
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

IOTaskManager::IOTaskManager(StorageManager& storageManager):storageManager_(storageManager) {
}

void IOTaskManager::init(uint16_t maxThreads, uint32_t maxSockets, const char *logName) {
  //TODO: set conectionTimeout from config
  connectionTimeout_ = 100;
  logger = Logger::getInstance(logName);

  maxThreads_ = maxThreads;
  maxSockets_ = maxSockets;
  pool_.setMaxThreads(maxThreads_);
  taskSorter_.init(maxThreads_);

  for (uint16_t i = 1; i <= maxThreads_; ++i) {
    IOTask *t = newTask();
    taskSorter_.assignTask(i, t);
    pool_.startTask(t);
  }
}

void IOTaskManager::removeContext(IOTask* t, uint16_t contextsNumber) {
  MutexGuard g(tasksMutex_);
  t->itemsCount_ -= contextsNumber;
  taskSorter_.reorderTask(t);
}

inline void IOTaskManager::giveContext(IOTask *t, ConnectionContext* cx) {
  smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketsCount(), cx);

  t->itemsCount_++;
  t->registerContext(cx);
  taskSorter_.reorderTask(t);
}

bool IOTaskManager::serverReady() {
  MutexGuard g(tasksMutex_);
  IOTask *t = (IOTask*)taskSorter_.getFirst();
  return t->getSocketsCount() < maxSockets_;
}

bool IOTaskManager::process(ConnectionContext* cx) {
  MutexGuard g(tasksMutex_);
  IOTask *t = (IOTask*)taskSorter_.getFirst();
  if (t->getSocketsCount() < maxSockets_) {
    cx->socket->Write("OK", 2);
    giveContext(t, cx);
    return true;
  } else {
    smsc_log_warn(logger, "Can't process %p context. Server busy.", cx);
    return false;
  }
}

bool IOTaskManager::storageProcess(ConnectionContext* cx) {
  return storageManager_.process(cx);
}

IOTask* IOTaskManager::newTask() {
  return new IOTask(*this, connectionTimeout_);
}

bool IOTaskManager::canStop() {
  MutexGuard g(tasksMutex_);
  IOTask *task = (IOTask*)taskSorter_.getTask(maxThreads_);
  return task->getSocketsCount() == 0;
}

void IOTaskManager::shutdown() {
  pool_.shutdown();
  storageManager_.shutdown();
}

}//mtpers
}//scag

