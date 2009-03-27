#ifndef _SCAG_UTIL_STORAGE_DATAFILEMANAGER_H_
#define _SCAG_UTIL_STORAGE_DATAFILEMANAGER_H_

#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/Mutex.hpp"

namespace scag    {
namespace util    {
namespace storage {

using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;

class DataFileCreator;

class DataFileTask : public ThreadedTask {
public:
  DataFileTask(DataFileCreator& creator):creator_(creator) {};
  int Execute();
  const char * taskName();
private:
  DataFileCreator& creator_;
};

class DataFileManager {
public:
  DataFileManager(uint16_t maxThreads):maxThreads_(maxThreads) { };
  void createDataFile(DataFileCreator& creator);
private:
  uint16_t maxThreads_;
  ThreadPool pool_;
};

}
}
}

#endif

