#ifndef _SCAG_PVSS_PVSS_PVSSDISPATCHER_H_
#define _SCAG_PVSS_PVSS_PVSSDISPATCHER_H_

#include <vector>
#include <memory>
#include "logger/Logger.h"
#include "core/buffers/Array.hpp"

#include "scag/pvss/api/core/server/Dispatcher.h"
//#include "scag/pvss/api/core/PvssSocket.h"
#include "scag/pvss/pvss/PvssLogic.h"

namespace scag2 {
namespace pvss  {

namespace core {
namespace server {
class ServerCore;
}
}

using smsc::logger::Logger;
using smsc::core::buffers::Array;
using std::vector;

struct NodeConfig;
class Request;
class scag::util::storage::DataFileManager;

class PvssDispatcher : public core::server::SyncDispatcher {
public:
  PvssDispatcher(const NodeConfig& nodeCfg);
  virtual ~PvssDispatcher();

  virtual unsigned getIndex(Request& request) const;
  virtual Server::SyncLogic* getSyncLogic(unsigned idx);
  virtual void reportStatistics() const;

  void init( core::server::ServerCore* serverCore, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg) throw (smsc::util::Exception);
  unsigned getInfrastructNodeNumber() const { return 0; }

    inline unsigned getNodeNumber() const { return nodeNumber_; }
    inline unsigned getStoragesCount() const { return storagesCount_; }

  unsigned getLocationNumber(unsigned elementStorageNumber) const;

private:
  AbonentLogic* getLocation(unsigned elementStorageNumber);
  void shutdown();

private:
  unsigned getErrorIndex() const { return static_cast<unsigned>(-1); }

private:
  unsigned nodeNumber_;
  unsigned locationsCount_;
  unsigned storagesCount_;
  uint16_t createdLocations_;
  unsigned infrastructIndex_;
  std::auto_ptr<InfrastructLogic> infrastructLogic_;
  Logger *logger_;
  Array<AbonentLogic*> abonentLogics_;
  scag::util::storage::DataFileManager dataFileManager_;
};

struct NodeConfig {
  NodeConfig():storagesCount(100), nodesCount(1), nodeNumber(0), locationsCount(0) {};
  unsigned storagesCount;
  unsigned nodesCount;
  unsigned nodeNumber;
  unsigned locationsCount;
};


}//pvss
}//scag2

#endif
 

