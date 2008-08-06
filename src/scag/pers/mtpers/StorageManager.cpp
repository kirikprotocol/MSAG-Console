#include "StorageManager.h"

namespace scag { namespace mtpers {

void StorageManager::init(uint16_t maxWaitingCount, uint16_t storageNumber, const StorageConfig& cfg) {
  //TODO: init storages;
  //need storage name and storage path
  storageNumber_ = storageNumber;
  for (int i = 0; i < storageNumber_; ++i) {
    StorageProcessor* proc = new AbonentStorageProcessor(maxWaitingCount, i);
    proc->init(cfg);
    storages_.Push(proc);
    pool_.startTask(proc);
  }
  notAbonentsStorage_ = new InfrastructStorageProcessor(maxWaitingCount);
  notAbonentsStorage_->init(cfg);
  pool_.startTask(notAbonentsStorage_);
}

bool StorageManager::process(ConnectionContext* cx) {
  if (cx->packet->notAbonentsProfile()) {
    smsc_log_debug(logger_, "give %p context to not abonent's processor", cx);
    return notAbonentsStorage_->addContext(cx);
  } else {
    uint16_t storageIndex = cx->packet->address.getNumber() % storageNumber_;
    smsc_log_debug(logger_, "give %p context to %d processor", cx, storageIndex);
    return storages_[storageIndex]->addContext(cx);
  }
}

void StorageManager::shutdown() {
  pool_.shutdown();
}


}//mtpers
}//scag
