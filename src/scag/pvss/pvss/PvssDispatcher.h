#ifndef _SCAG_PVSS_PVSS_PVSSDISPATCHER_H_
#define _SCAG_PVSS_PVSS_PVSSDISPATCHER_H_

#include <vector>
#include <memory>
#include "logger/Logger.h"
#include "core/buffers/Array.hpp"

#include "scag/pvss/api/core/server/Dispatcher.h"
#include "scag/pvss/pvss/PvssLogic.h"

namespace scag {
namespace util {
namespace storage {
class DataFileManager;
}
}
}

namespace scag2 {
namespace pvss  {

using smsc::logger::Logger;
using smsc::core::buffers::Array;
using std::vector;

struct NodeConfig {
  NodeConfig():storagesCount(100), nodesCount(1), nodeNumber(0), locationsCount(0), disksCount(0), expectedSpeed(100) {};
  unsigned storagesCount;
  unsigned nodesCount;
  unsigned nodeNumber;
  unsigned locationsCount;
  unsigned disksCount;
  unsigned expectedSpeed;
};

class Request;

class PvssDispatcher : public core::server::SyncDispatcher {
public:
  PvssDispatcher(const NodeConfig& nodeCfg);
  virtual ~PvssDispatcher();

  virtual unsigned getIndex(Request& request) const;
  virtual Server::SyncLogic* getSyncLogic(unsigned idx);
    virtual std::string reportStatistics() const;

    void createLogics( bool makedirs,
                       const AbonentStorageConfig& abntcfg,
                       const InfrastructStorageConfig* infcfg) /* throw (smsc::util::Exception) */;

    void init();
    void rebuildIndex();

  unsigned getInfrastructNodeNumber() const { return 0; }

    inline unsigned getNodeNumber() const { return nodeCfg_.nodeNumber; }
    inline unsigned getStoragesCount() const { return nodeCfg_.storagesCount; }

  unsigned getLocationNumber(unsigned elementStorageNumber) const;

private:
  AbonentLogic* getLocation(unsigned elementStorageNumber);
  void shutdown();

private:
  unsigned getErrorIndex() const { return static_cast<unsigned>(-1); }

private:
  NodeConfig nodeCfg_;
  uint16_t createdLocations_;
  unsigned infrastructIndex_;
  std::auto_ptr<InfrastructLogic> infrastructLogic_;
  Logger *logger_;
  Array<AbonentLogic*> abonentLogics_;
  vector<scag::util::storage::DataFileManager*> dataFileManagers_;
};

}//pvss
}//scag2

#endif
 

