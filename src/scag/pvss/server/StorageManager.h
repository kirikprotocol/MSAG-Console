#ifndef _SCAG_PVSS_SERVER_STORAGE_MANAGER_H_
#define _SCAG_PVSS_SERVER_STORAGE_MANAGER_H_


#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "logger/Logger.h"

#include "StorageProcessor.h"

namespace scag2 {
namespace pvss  {

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
  virtual ~StorageManager();

  void init(uint16_t maxWaitingCount, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg);
  bool process(PersPacket* packet);
  void shutdown();
  unsigned getInfrastructNodeNumber() { return 0; }
  void procStopped();

private:
  AbonentStorageProcessor* getLocation(unsigned elementStorageNumber);
  bool allprocessorsStopped();

private:
  ThreadPool pool_;
  Mutex procMutex_;
  uint16_t startedProc_;
  bool isStopped_;
  Logger *logger_;
  Array<AbonentStorageProcessor*> storages_;
  unsigned locationsCount_;
  unsigned storagesCount_;
  unsigned nodeNumber_;
  InfrastructStorageProcessor *infrastructStorage_;

};

}//pvss
}//scag2

#endif
 

