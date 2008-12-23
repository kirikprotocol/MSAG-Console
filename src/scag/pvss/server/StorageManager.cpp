#include "StorageManager.h"
#include "scag/util/storage/StorageNumbering.h"

namespace scag2 {
namespace pvss  {

using scag::util::storage::StorageNumbering;

StorageManager::StorageManager(const NodeConfig& nodeCfg): nodeNumber_(nodeCfg.nodeNumber), locationsCount_(nodeCfg.locationsCount), isStopped_(true),
                                                           storagesCount_(nodeCfg.storagesCount), logger_(Logger::getInstance("storeman")), startedProc_(0) {
  StorageNumbering::setInstance(nodeCfg.nodesCount);
}

void StorageManager::init(uint16_t maxWaitingCount, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg) {
  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    if (!File::Exists(abntcfg.locationPath[locationNumber].c_str())) {
      smsc_log_debug(logger_, "create storage dir '%s'", abntcfg.locationPath[locationNumber].c_str());
      File::MkDir(abntcfg.locationPath[locationNumber].c_str());
    }
    AbonentStorageProcessor* proc = new AbonentStorageProcessor(maxWaitingCount, locationNumber, storagesCount_, this);
    storages_.Push(proc);
  }
  for (unsigned i = 0; i < storagesCount_; ++i) {
    if (StorageNumbering::instance().node(i) == nodeNumber_) {
      getLocation(i)->initElementStorage(abntcfg, i);
    }
  }
  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    pool_.startTask(storages_[locationNumber]);
    ++startedProc_;
  }
  if (nodeNumber_ == getInfrastructNodeNumber()) {
    if (!infcfg) {
      throw Exception("Erorr init infrastruct storage, config in NULL");
    }
    infrastructStorage_ = new InfrastructStorageProcessor(maxWaitingCount, this);
    infrastructStorage_->init(*infcfg);
    pool_.startTask(infrastructStorage_);
    ++startedProc_;
  }

  isStopped_ = false;

}

AbonentStorageProcessor* StorageManager::getLocation(unsigned elementStorageNumber) {
  return storages_[((elementStorageNumber / StorageNumbering::instance().nodes()) % locationsCount_)];
}

void StorageManager::procStopped() {
  MutexGuard mg(procMutex_);
  --startedProc_;
}

bool StorageManager::allprocessorsStopped() {
  MutexGuard mg(procMutex_);
  return startedProc_ > 0 ? false : true;
}

bool StorageManager::process(PersPacket* packet) {
  if (isStopped_) {
    return false;
  }
  if (packet->notAbonentsProfile()) {
    if (nodeNumber_ != getInfrastructNodeNumber()) {
      smsc_log_warn(logger_, "can't process infrastruct request on node number=%d", nodeNumber_);
      return false;
    }
    smsc_log_debug(logger_, "give %p packet to not abonent's processor", packet);
    return infrastructStorage_->addPacket(packet);
  } else {
    unsigned storageNumber = 0;
    try {
      storageNumber = static_cast<unsigned>(packet->address.getNumber() % storagesCount_);
      smsc_log_debug(logger_, "give %p packet to storage %d in node %d", packet, storageNumber, nodeNumber_);
      return getLocation(storageNumber)->addPacket(packet);      
    } catch (const std::runtime_error& e) {
      smsc_log_warn(logger_, "Can't find location for elementStorage %d on node %d. std::runtime_error: %s", storageNumber, nodeNumber_, e.what());
      return false;
    }
  }
}

void StorageManager::shutdown() {
  isStopped_ = true;
  infrastructStorage_->stop();
  for (unsigned i = 0; i < locationsCount_; ++i) {
    storages_[i]->stop();
  }
  while (!allprocessorsStopped()) {
    smsc_log_warn(logger_, "waiting 5 seconds while storage processors stopping...");
    sleep(5);
  }
  smsc_log_warn(logger_, "shutdown storage processors pool");
  pool_.shutdown();
}

StorageManager::~StorageManager() {
  if (!isStopped_) {
    shutdown();
  }
}


}//pvss
}//scag2
