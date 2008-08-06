#ifndef __SCAG_MTPERS_STORAGE_MANGER_H__
#define __SCAG_MTPERS_STORAGE_MANGER_H__


#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "logger/Logger.h"

#include "ConnectionContext.h"
#include "StorageProcessor.h"

namespace scag { namespace mtpers {

using smsc::core::threads::ThreadPool;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
using smsc::core::buffers::Array;

class StorageManager {
public:
  StorageManager():storageNumber_(0), logger_(Logger::getInstance("storeman")) {};
  virtual ~StorageManager() {};

  void init(uint16_t maxWaitingCount, uint16_t storageNumber, const StorageConfig& cfg);
  bool process(ConnectionContext* cx);
  void shutdown();

private:
  ThreadPool pool_;
  Mutex procMutex_;
  Logger *logger_;
  uint16_t storageNumber_;
  Array<StorageProcessor *> storages_;
  StorageProcessor* notAbonentsStorage_;

};

}//mtpers
}//scag

#endif
 

