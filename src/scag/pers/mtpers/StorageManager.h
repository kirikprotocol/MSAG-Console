#ifndef __SCAG_MTPERS_STORAGE_MANGER_H__
#define __SCAG_MTPERS_STORAGE_MANGER_H__


#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "logger/Logger.h"

#include "StorageProcessor.h"

namespace scag { namespace mtpers {

using smsc::core::threads::ThreadPool;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
using smsc::core::buffers::Array;
using std::vector;

struct NodeConfig {
  NodeConfig():storagesCount(100), nodesCount(1), nodeNumber(0), locationsCount(0) {};
  unsigned storagesCount;
  unsigned nodesCount;
  unsigned nodeNumber;
  unsigned locationsCount;
};

class StorageManager {
public:
  StorageManager(const NodeConfig& nodeCfg);
  virtual ~StorageManager() {};

  void init(uint16_t maxWaitingCount, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg);
  bool process(PersPacket* cx);
  void shutdown();
  unsigned getInfrastructNodeNumber() { return 0; }

private:
  AbonentStorageProcessor* getLocation(unsigned elementStorageNumber);

private:
  ThreadPool pool_;
  Mutex procMutex_;
  Logger *logger_;
  Array<AbonentStorageProcessor*> storages_;
  unsigned locationsCount_;
  unsigned storagesCount_;
  unsigned nodeNumber_;
  InfrastructStorageProcessor *infrastructStorage_;

};

}//mtpers
}//scag

#endif
 

