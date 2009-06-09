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
    DataFileManager(uint16_t maxThreads,
                    unsigned addSpeed,
                    unsigned freeCountThreshold = 5000 ) :
    maxThreads_(maxThreads), addSpeed_(addSpeed),
    threshold_(freeCountThreshold) {
        pool_.setMaxThreads(maxThreads_);
        pool_.preCreateThreads(maxThreads_);
    };

  void createDataFile(DataFileCreator& creator);
  unsigned getExpectedSpeed() const {
    return addSpeed_;
  }

    inline void shutdown() { pool_.shutdown(); }

    /// when to start new file creation (in free blocks)
    inline unsigned creationThreshold() const { return threshold_; }

    /// used eg for initialization
    void startTask( ThreadedTask* task, bool delOnCompletion = true );

private:
  uint16_t maxThreads_;
  unsigned addSpeed_;
    unsigned threshold_;   // when to start file creation (in free blocks)
  ThreadPool pool_;
};

}
}
}

namespace scag2 {
namespace util {
namespace storage {
using namespace scag::util::storage;
}
}
}

#endif

