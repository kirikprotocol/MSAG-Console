#include "PvssDispatcher.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"

namespace scag2 {
namespace pvss  {

using scag::util::storage::StorageNumbering;

PvssDispatcher::PvssDispatcher(const NodeConfig& nodeCfg): nodeNumber_(nodeCfg.nodeNumber), locationsCount_(nodeCfg.locationsCount),
                                                           storagesCount_(nodeCfg.storagesCount), createdLocations_(0), infrastructIndex_(storagesCount_),
                                                           logger_(Logger::getInstance("pvss.disp")){
  StorageNumbering::setInstance(nodeCfg.nodesCount);
}

unsigned PvssDispatcher::getIndex(Request& request) {

  AbstractProfileRequest& profileRequest = static_cast<AbstractProfileRequest&>(request);
  const ProfileKey& profileKey = profileRequest.getProfileKey();

  if (profileKey.hasOperatorKey() || profileKey.hasProviderKey() || profileKey.hasServiceKey()) {
    if (nodeNumber_ != getInfrastructNodeNumber()) {
      smsc_log_warn(logger_, "can't process infrastruct request on node number=%d", nodeNumber_);
      return getErrorIndex();
    }
    smsc_log_debug(logger_, "give %p packet to infrastruct storage node %d", &request, getInfrastructNodeNumber());
    return infrastructIndex_;
  }

  unsigned storageNumber = static_cast<unsigned>(profileKey.getAddress().getNumber() % storagesCount_);
  smsc_log_debug(logger_, "give %p packet to storage %d in node %d", &request, storageNumber, nodeNumber_);
  return getLocationNumber( storageNumber );
}

Server::SyncLogic* PvssDispatcher::getSyncLogic(unsigned idx) {
  if (getErrorIndex() == idx || idx > locationsCount_) {
    return 0;
  }
  if (idx == infrastructIndex_) {
    return infrastructLogic_.get();
  }
  //return getLocation(idx);
  return abonentLogics_[idx];
}

void PvssDispatcher::init(const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg) {

  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    if (!File::Exists(abntcfg.locationPath[locationNumber].c_str())) {
      smsc_log_debug(logger_, "create storage dir '%s'", abntcfg.locationPath[locationNumber].c_str());
      File::MkDir(abntcfg.locationPath[locationNumber].c_str());
    }
    AbonentLogic* logic = new AbonentLogic(locationNumber, storagesCount_);
    abonentLogics_.Push(logic);
    ++createdLocations_;
  }
  for (unsigned i = 0; i < storagesCount_; ++i) {
    if (StorageNumbering::instance().node(i) == nodeNumber_) {
      getLocation(i)->initElementStorage(abntcfg, i);
    }
  }
  if (nodeNumber_ == getInfrastructNodeNumber()) {
    if (!infcfg) {
      throw Exception("Erorr init infrastruct storage, config in NULL");
    }
    infrastructLogic_.reset( new InfrastructLogic );
    infrastructLogic_->init(*infcfg);
  }
}

unsigned PvssDispatcher::getLocationNumber(unsigned elementStorageNumber) {
  return (elementStorageNumber / StorageNumbering::instance().nodes()) % locationsCount_;
}

AbonentLogic* PvssDispatcher::getLocation(unsigned elementStorageNumber) {
  return abonentLogics_[getLocationNumber(elementStorageNumber)];
}

void PvssDispatcher::shutdown() {
  uint16_t created = createdLocations_;
  for (unsigned i = 0; i < created; ++i) {
    delete abonentLogics_[i];
    --createdLocations_;
  }
}

PvssDispatcher::~PvssDispatcher() {
  shutdown();
}


}//pvss
}//scag2
