/* $Id$ */

#ifndef SCAG_PERS_CENTRAL_PERSSERVER
#define SCAG_PERS_CENTRAL_PERSSERVER

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "PersSocketServer.h"
#include "FSDB.h"
#include "mcisme/AbntAddr.hpp"
#include "core/buffers/XHash.hpp"
#include "core/buffers/IntHash.hpp"

#include "XMLHandlers.h"
#include <xercesc/sax/HandlerBase.hpp>

#include "Types.h"
#include "CPersCmd.h"

namespace scag { namespace cpers {

XERCES_CPP_NAMESPACE_USE

using namespace scag::pers;
using smsc::mcisme::AbntAddr;
using smsc::logger::Logger;

class ProfileInfo
{
public:
    uint32_t owner;
};

class TransactionInfo
{
public:
  TransactionInfo(): startTime(time(NULL)) {};
  uint32_t owner, candidate, startTime;
};

class CentralPersServer : public PersSocketServer {
public:
  CentralPersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, const std::string& dbPath, const std::string& dbName, uint32_t indexGrowth, const char* regionsFileName);
  virtual bool processPacket(ConnectionContext& ctx);

protected:
  Logger *logger;
  typedef RBTreeHSAllocator<AbntAddr, ProfileInfo> ProfileInfoAllocator;
  typedef RBTree<AbntAddr, ProfileInfo> ProfileInfoStorage;
  ProfileInfoAllocator profileInfoAllocator;
  ProfileInfoStorage profileInfoStore;
  smsc::core::buffers::XHash<AbntAddr, TransactionInfo, AbntAddr> transactions;
  Mutex regionsReloadMutex, regionsMapMutex;
  IntHash<RegionInfo>* regions;
  bool getRegionInfo(uint32_t id, RegionInfo& ri);
  bool getProfileInfo(std::string& key, ProfileInfo& pi);
  void reloadRegions(const char* regionsFileName);
  void ParseFile(const char* _xmlFile, HandlerBase* handler);

private:
  void getProfileCmdHandler(ConnectionContext& ctx);
  void profileRespCmdHandler(ConnectionContext& ctx);
  void doneCmdHandler(ConnectionContext& ctx);
  void doneRespCmdHandler(ConnectionContext& ctx);
  void checkOwnCmdHandler(ConnectionContext& ctx);
  void sendCommand(CPersCmd& cmd, ConnectionContext* ctx);
  bool authorizeRegion(ConnectionContext& ctx);
};

}}

#endif
