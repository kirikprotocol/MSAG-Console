#include "StorageManager.h"
#include "scag/util/storage/StorageNumbering.h"

namespace scag { namespace mtpers {

using scag::util::storage::StorageNumbering;

StorageManager::StorageManager(const NodeConfig& nodeCfg): nodeNumber_(nodeCfg.nodeNumber), locationsCount_(nodeCfg.locationsCount),
                                                           storagesCount_(nodeCfg.storagesCount), logger_(Logger::getInstance("storeman")) {
  StorageNumbering::setInstance(nodeCfg.nodesCount);
  //storagesCount_ = StorageNumbering::instance().storages();
}

void StorageManager::init(uint16_t maxWaitingCount, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig& infcfg) {
  vector<string> locationPath(abntcfg.locationPath);
  if (locationPath.empty()) {
    smsc_log_warn(logger_, "locations paths not set, set locations count=1 location path='./storage/abonent'");
    locationPath.push_back("./storage/abonent");
  }
  locationsCount_ = locationPath.size();
  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    if (!File::Exists(locationPath[locationNumber].c_str())) {
      smsc_log_debug(logger_, "create storage dir '%s'", locationPath[locationNumber].c_str());
      File::MkDir(locationPath[locationNumber].c_str());
    }
    AbonentStorageProcessor* proc = new AbonentStorageProcessor(maxWaitingCount, locationNumber, storagesCount_);
    storages_.Push(proc);
  }
  for (unsigned i = 0; i < storagesCount_; ++i) {
    if (StorageNumbering::instance().node(i) == nodeNumber_) {
      getLocation(i)->initElementStorage(abntcfg, i);
    }
  }
  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    pool_.startTask(storages_[locationNumber]);
  }

  infrastructStorage_ = new InfrastructStorageProcessor(maxWaitingCount);
  infrastructStorage_->init(infcfg);
  pool_.startTask(infrastructStorage_);
}

AbonentStorageProcessor* StorageManager::getLocation(unsigned elementStorageNumber) {
  return storages_[((elementStorageNumber / StorageNumbering::instance().nodes()) % locationsCount_)];
}

bool StorageManager::process(ConnectionContext* cx) {
  if (cx->packet->notAbonentsProfile()) {
    smsc_log_debug(logger_, "give %p context to not abonent's processor", cx);
    return infrastructStorage_->addContext(cx);
  } else {
    unsigned storageNumber = 0;
    try {
      storageNumber = cx->packet->address.getNumber() % storagesCount_;
      smsc_log_debug(logger_, "give %p context to storage %d in node %d", cx, storageNumber, nodeNumber_);
      return getLocation(storageNumber)->addContext(cx);      
    } catch (const std::runtime_error& e) {
      smsc_log_warn(logger_, "Can't find location for elementStorage %d on node %d. std::runtime_error: %s", storageNumber, nodeNumber_, e.what());
      return false;
    }
  }
}

void StorageManager::shutdown() {
  pool_.shutdown();
}

}//mtpers
}//scag
