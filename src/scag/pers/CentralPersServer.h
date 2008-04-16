/* $Id$ */

#ifndef SCAG_PERS_CENTRAL_PERSSERVER
#define SCAG_PERS_CENTRAL_PERSSERVER

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "PersSocketServer.h"
#include "FSDB.h"
#include "AbntAddr.hpp"
#include "core/buffers/XHash.hpp"
#include "core/buffers/IntHash.hpp"

#include "XMLHandlers.h"
#include <xercesc/sax/HandlerBase.hpp>

#include "Types.h"
#include "CPersCmd.h"

namespace scag { namespace cpers {

XERCES_CPP_NAMESPACE_USE

using namespace scag::pers;
using scag::pers::AbntAddr;
using smsc::logger::Logger;

class ProfileInfo
{
public:
    uint32_t owner;
    ProfileInfo() : owner(0) {};
    operator int() const {
      return owner;
    }
};

class TransactionInfo
{
public:
  TransactionInfo(): startTime(time(NULL)), wait_cmd(CentralPersCmd::PROFILE_RESP),
                     owner(0), candidate(0) {};
  TransactionInfo(uint32_t _owner, uint32_t _candidate)
   :startTime(time(NULL)), owner(_owner), candidate(_candidate), wait_cmd(CentralPersCmd::PROFILE_RESP) 
  {
  };
  uint32_t owner, candidate;
  time_t startTime;
  uint8_t wait_cmd;
};

class CentralPersServer : public PersSocketServer {
public:
  CentralPersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, int transactTimeout_,
                    const std::string& dbPath, const std::string& dbName, uint32_t indexGrowth, const char* regionsFileName);
  virtual bool processPacket(ConnectionContext& ctx);

protected:
  Logger *logger;
  Logger *info_log;
  typedef RBTreeHSAllocator<AbntAddr, ProfileInfo> ProfileInfoAllocator;
  typedef RBTree<AbntAddr, ProfileInfo> ProfileInfoStorage;
  ProfileInfoAllocator profileInfoAllocator;
  ProfileInfoStorage profileInfoStore;
  smsc::core::buffers::XHash<AbntAddr, TransactionInfo, AbntAddr> transactions;
  Mutex regionsReloadMutex, regionsMapMutex;
  IntHash<RegionInfo>* regions;
  bool getRegionInfo(uint32_t id, RegionInfo& ri);
  bool getProfileInfo(AbntAddr& key, ProfileInfo& pi);
  void reloadRegions(const char* regionsFileName);
  void ParseFile(const char* _xmlFile, HandlerBase* handler);
  void checkTransactionsTimeouts();
  void onDisconnect(ConnectionContext& ctx);  

private: 
  void execCommand(ConnectionContext& ctx);
  void getProfileCmdHandler(ConnectionContext& ctx);
  void profileRespCmdHandler(ConnectionContext& ctx);
  void doneCmdHandler(ConnectionContext& ctx);
  //void doneRespCmdHandler(ConnectionContext& ctx);
  void checkOwnCmdHandler(ConnectionContext& ctx);
  void pingCmdHandler(ConnectionContext& ctx);
  void sendCommand(const CPersCmd& cmd, ConnectionContext* ctx);
  void sendResponse(const CPersCmd& cmd, ConnectionContext& ctx);
  void sendCommand(const CPersCmd& cmd, uint32_t region_id);
  bool authorizeRegion(ConnectionContext& ctx);
  void transactionTimeout(const AbntAddr& addr, const TransactionInfo& tr_info);

};

}}

#endif
