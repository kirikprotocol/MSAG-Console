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

using smsc::logger::Logger;
using smsc::core::buffers::Array;
using std::vector;

struct NodeConfig;
class Request;

class PvssDispatcher : public core::server::SyncDispatcher {
public:
  PvssDispatcher(const NodeConfig& nodeCfg);
  virtual ~PvssDispatcher();

  virtual unsigned getIndex(Request& request);
  virtual Server::SyncLogic* getSyncLogic(unsigned idx);

  void init(const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg);
  unsigned getInfrastructNodeNumber() const { return 0; }

private:
  unsigned getLocationNumber(unsigned elementStorageNumber);
  AbonentLogic* getLocation(unsigned elementStorageNumber);
  void shutdown();

private:
  unsigned getErrorIndex() const { return static_cast<unsigned>(-1); }

private:
  unsigned locationsCount_;
  unsigned storagesCount_;
  unsigned nodeNumber_;
  unsigned infrastructIndex_;
  uint16_t createdLocations_;
  std::auto_ptr<InfrastructLogic> infrastructLogic_;
  Logger *logger_;
  Array<AbonentLogic*> abonentLogics_;
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
 

