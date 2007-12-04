/* $Id$ */

#include "CentralPersServer.h"
#include "scag/exc/SCAGExceptions.h"

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include "XMLHandlers.h"

namespace scag { namespace cpers {

using namespace scag::exceptions;
using smsc::util::Exception;
using smsc::logger::Logger;

const int TRANSACT_CHECK_PERIOD = 100;

void CentralPersServer::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
    SAXParser parser;
    
    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(handler);
        parser.setErrorHandler(handler);
        parser.parse(_xmlFile);
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing: XMLPlatform: OutOfMemoryException");
        throw Exception("XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing: XMLException: %s", msg.localForm());
        throw Exception("XMLException: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("unknown fatal error");
    }
}

void CentralPersServer::reloadRegions(const char* regionsFileName)
{
    MutexGuard mt(regionsReloadMutex);
    
    smsc_log_info(logger, "ReloadRegions Started");

    IntHash<RegionInfo> *hash = new IntHash<RegionInfo>();
    try{
        XMLBasicHandler handler(hash);
        ParseFile(regionsFileName, &handler);

        MutexGuard mt1(regionsMapMutex);
        delete regions;
        regions = hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Regions reload was not successful");
        delete hash;
        throw;
    }

    smsc_log_info(logger, "ReloadRegions Finished");
}

CentralPersServer::CentralPersServer(const char* persHost_, int persPort_, int maxClientCount_,
                                     int timeout_, const std::string& dbPath, const std::string& dbName,
                                     uint32_t indexGrowth, const char* regionsFileName):
 PersSocketServer(persHost_, persPort_, maxClientCount_, timeout_), last_check_time(time(NULL)),
   logger(Logger::getInstance("cpersserv")), regions(NULL)
{
    reloadRegions(regionsFileName);
        
   	if(profileInfoAllocator.Init(dbPath + '/' + dbName, indexGrowth))
        throw SCAGException("Init ProfileInfoStore failed: dbPath=%s, dbName=%s", dbPath.c_str(), dbName.c_str());
        
   	profileInfoStore.SetAllocator(&profileInfoAllocator);
   	profileInfoStore.SetChangesObserver(&profileInfoAllocator);
}

bool CentralPersServer::getRegionInfo(uint32_t id, RegionInfo& ri)
{
    MutexGuard mt(regionsMapMutex);
    RegionInfo* pri = regions->GetPtr(id);
    if(!pri) return false;
    if (!pri->ctx) {
      smsc_log_warn(logger, "getRegionInfo: ConnectionContext of region id=%d is empty", pri->id);
    }
    ri = *pri;
    return true;
}

bool CentralPersServer::getProfileInfo(AbntAddr& key, ProfileInfo& pi)
{
    return profileInfoStore.Get(key, pi);
}

bool CentralPersServer::authorizeRegion(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf;
  uint32_t id = isb.ReadInt32();
  std::string psw;
  isb.ReadString(psw);
  //LoginCmd login_cmd(isb);
  //uint32_t id = login_cmd.rp_id;
  //std::string psw = login_cmd.rp_psw;
  smsc_log_debug(logger, "RP id=%d pswd=%s", id, psw.c_str());
  MutexGuard mt(regionsMapMutex);
  RegionInfo* pri = regions->GetPtr(id);
  if (!pri || !(ctx.authed = !strncmp(pri->passwd.c_str(), psw.c_str(), 30))) {
    return false;
  }
  ctx.region_id = id;
  pri->ctx = &ctx;
  return true;
}

bool CentralPersServer::processPacket(ConnectionContext& ctx)
{
    SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;

    try {
        uint8_t cmd = isb.ReadInt8();
        smsc_log_debug(logger, "cmd=%d", cmd);
        if(cmd == CentralPersCmd::LOGIN)
        {
            return authorizeRegion(ctx);
        }
        else if(!ctx.authed) {
          smsc_log_warn(logger, "RP not authorized");
          return false;
        }
        smsc_log_debug(logger, "RP authorized");
   
        //std::string key;    
        //isb.ReadString(key);
        //smsc_log_debug(logger, "key=\'%s\'", key.c_str());
        //ProfileInfo profileInfo;
        //TransactionInfo transactInfo, *pti;
        //AbntAddr addr(key.c_str());
        //bool profile_exist = profileInfoStore.Get(addr, profileInfo);
        //if(pti = transactions.GetPtr(addr))
            //transactInfo = *pti;
        switch(cmd)
        {
            case CentralPersCmd::GET_PROFILE:
              smsc_log_debug(logger, "GET_PROFILE Profile received");
              getProfileCmdHandler(ctx);
              break;
            case CentralPersCmd::PROFILE_RESP:
              smsc_log_debug(logger, "PROFILE Resp received");
              profileRespCmdHandler(ctx);
              break;
            case CentralPersCmd::DONE_RESP:
              smsc_log_debug(logger, "DONE_RESP received");
              doneRespCmdHandler(ctx);
              break;
            case CentralPersCmd::DONE:
              smsc_log_debug(logger, "DONE received");
              doneCmdHandler(ctx);
              break;
            case CentralPersCmd::CHECK_OWN:
              //checkOwnCmdHandler(ctx);
              smsc_log_debug(logger, "CHECK_OWN received");
              break;
        }
    }
    catch(...)
    {
      smsc_log_warn(logger, "Exception in processPacket ctx=%p", &ctx);
    }
    return true;
}

void CentralPersServer::getProfileCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "getProfileCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  GetProfileCmd get_profile(isb);
  smsc_log_debug(logger, "getProfileCmdHandler: profile key = \'%s\'", get_profile.key.c_str());
  ProfileInfo profile_info;
  AbntAddr addr(get_profile.key.c_str());
  addr.setNumberingPlan(1);
  addr.setTypeOfNumber(1);
  if (!getProfileInfo(addr, profile_info)) {
    smsc_log_debug(logger, "getProfileCmdHandler: profile key = \'%s\' not registered", get_profile.key.c_str());
    profile_info.owner = ctx.region_id;
    profileInfoStore.Insert(addr, profile_info);
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.serialize(osb);
    return;
  }
  if (profile_info.owner == ctx.region_id) {
    smsc_log_warn(logger, "getProfileCmdHandler: region id=%d is now profile \'%s\' owner",
                  ctx.region_id, get_profile.key.c_str());
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.serialize(osb);
  }
  smsc_log_debug(logger, "getProfileCmdHandler: profile key=\'%s\' registered owner=%d",
                  get_profile.key.c_str(), profile_info.owner);
  RegionInfo ri;
  if (!getRegionInfo(profile_info.owner, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ? "unavailable" : "not found";
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d %s",
                   profile_info.owner, err_msg.c_str());
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.is_ok = 0;
    profile_resp.serialize(osb);
    return;
  }
  TransactionInfo tr_info;
  tr_info.owner = profile_info.owner;
  tr_info.candidate = ctx.region_id;
  transactions.Insert(addr, tr_info);
  smsc_log_debug(logger, "getProfileCmdHandler: create transaction key = \'%s\' owner=%d candidate=%d",
                  get_profile.key.c_str(), tr_info.owner, tr_info.candidate);

  //get_profile.serialize(ri.ctx->outbuf);
  sendCommand(get_profile, ri.ctx);
}

void CentralPersServer::profileRespCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "profileRespCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  ProfileRespCmd profile_resp(isb);
  smsc_log_debug(logger, "profileRespCmdHandler: profile key = \'%s\'", profile_resp.key.c_str());
  AbntAddr addr(profile_resp.key.c_str());
  addr.setNumberingPlan(1);
  addr.setTypeOfNumber(1);
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "profileRespCmdHandler: Transcation with key=%s not found", profile_resp.key.c_str());
    DoneCmd done(0, profile_resp.key);
    done.serialize(osb);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->candidate, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ? "not found" : "unavailable";
    smsc_log_warn(logger, "profileRespCmdHandler: region with id=%d %s",
                   pti->candidate, err_msg.c_str());
    DoneCmd done(0, profile_resp.key);
    done.serialize(osb);
    smsc_log_debug(logger, "profileRespCmdHandler: delete transaction key=\'%s\'",
                    profile_resp.key.c_str());
    transactions.Delete(addr);
    return;
  }
  //profile_resp.serialize(ri.ctx->outbuf);
  sendCommand(profile_resp, ri.ctx);
  pti->last_cmd = CentralPersCmd::PROFILE_RESP;
  pti->startTime = time(NULL);
}

void CentralPersServer::doneCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "doneCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done(isb);
  smsc_log_debug(logger, "doneCmdHandler: profile key = \'%s\'", done.key.c_str());
  AbntAddr addr(done.key.c_str());
  addr.setNumberingPlan(1);
  addr.setTypeOfNumber(1);
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "doneCmdHandler: Transcation with key=%s not found", done.key.c_str());
    DoneRespCmd done_resp(0, done.key);
    done_resp.serialize(osb);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->owner, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ?  "unavailable" : "not found";;
    smsc_log_warn(logger, "doneCmdHandler: region with id=%d %s", pti->candidate, err_msg.c_str());
    DoneRespCmd done_resp(0, done.key);
    done_resp.serialize(osb);
    smsc_log_debug(logger, "doneCmdHandler: delete transaction key=\'%s\'", done.key.c_str());
    transactions.Delete(addr);
    return;
  }
  if (done.is_ok) {
    smsc_log_debug(logger, "doneCmdHandler : DONE OK change profile owner old=%d new=%d",
                    pti->owner, pti->candidate);
    ProfileInfo profile_info;
    profile_info.owner = pti->candidate;
    profileInfoStore.Set(addr, profile_info);
  }
  //done.serialize(ri.ctx->outbuf);
  sendCommand(done, ri.ctx);
  pti->last_cmd = CentralPersCmd::DONE;
  pti->startTime = time(NULL);
}

void CentralPersServer::doneRespCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "doneRespCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done_resp(isb);
  smsc_log_debug(logger, "doneRespCmdHandler: profile key = \'%s\'", done_resp.key.c_str());
  AbntAddr addr(done_resp.key.c_str());
  addr.setNumberingPlan(1);
  addr.setTypeOfNumber(1);
  TransactionInfo *pti;
  uint32_t owner = 0;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "doneRespCmdHandler: Transcation with key=%s not found", done_resp.key.c_str());
    ProfileInfo pi;
    if (profileInfoStore.Get(addr, pi)) {
      owner = pi.owner;
    } else {
      smsc_log_error(logger, "doneRespCmdHandler: can't find owner for profile=\'%s\'",
                     done_resp.key.c_str());
    }
  } else {
    owner = pti->candidate;
  }
  RegionInfo ri;
  if (!getRegionInfo(owner, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ?  "unavailable" : "not found";;
    smsc_log_warn(logger, "doneRespCmdHandler: region with id=%d %s", owner, err_msg.c_str());
    //smsc_log_debug(logger, "doneCmdHandler: delete transaction key=\'%s\'", done_resp.key.c_str());
  }
  if (!done_resp.is_ok) {
    smsc_log_warn(logger, "doneRespCmdHandler: receive error DONE_RESP profile \'%s\'",
                  done_resp.key.c_str());
    ProfileInfo pi;
    pi.owner = ctx.region_id;
    profileInfoStore.Set(addr, pi);
  }
  transactions.Delete(addr);
  //done_resp.serialize(ri.ctx->outbuf);
  sendCommand(done_resp, ri.ctx);
}

void CentralPersServer::sendCommand(CPersCmd& cmd, ConnectionContext* ctx) {
  if (!ctx) {
    return;
  }
  smsc_log_debug(logger, "sendCommand : cmd id=%d", cmd.cmd_id);
  cmd.serialize(ctx->outbuf);
  ctx->outbuf.SetPos(0);
  listener.addRW(ctx->socket);
}

void CentralPersServer::checkTimeouts() {
  PersSocketServer::checkTimeouts();
  checkTransactionsTimeouts();
}

void CentralPersServer::transactionTimeout(const AbntAddr& addr, const TransactionInfo& tr_info) {
  uint8_t last_cmd(tr_info.last_cmd);
  smsc_log_warn(logger, "transaction timeout key=\'%s\' last operation id=%d",
                 addr.toString().c_str(), last_cmd);
  //int addr_prefix_size = 5;
  string key(addr.toString(), ADDR_PREFIX_SIZE);
  switch (last_cmd) {
  case CentralPersCmd::GET_PROFILE: {
    ProfileRespCmd profile_resp(key);
    profile_resp.is_ok = 0;
    sendCommand(profile_resp, tr_info.candidate);
    break;
  }
  case CentralPersCmd::PROFILE_RESP: {
    DoneCmd done(0, key);
    sendCommand(done, tr_info.owner);
    transactions.Delete(addr);
    break;
  }
  case CentralPersCmd::DONE: 
    break;
    //не знаю чё делать если не пришёл DONE_RESP
  }
 // transactions.Delete(addr);

}

void CentralPersServer::sendCommand(CPersCmd &cmd, uint32_t region_id) {
  RegionInfo ri;
  if (!getRegionInfo(region_id, ri)) {
    smsc_log_error(logger, "sendCommand: region id=%d not found", region_id);
    return;
  }
  sendCommand(cmd, ri.ctx);
}

void CentralPersServer::checkTransactionsTimeouts() {
  //time_t time_bound = time(NULL) - TRANSACT_CHECK_PERIOD;
  //if (last_check_time > time_bound) {
    //return;
  //}
  time_t time_bound = time(NULL) - timeout / 2;
  AbntAddr key;
  TransactionInfo tr_info;
  transactions.First();
  while (transactions.Next(key, tr_info)) {
    if (tr_info.startTime < time_bound) {
      transactionTimeout(key, tr_info);
    }
  }
  last_check_time = time(NULL);
}

void CentralPersServer::onDisconnect(ConnectionContext& ctx) {
  MutexGuard mt(regionsMapMutex);
  RegionInfo* pri = regions->GetPtr(ctx.region_id);
  if (!pri) {
    return;
  }
  pri->ctx = NULL;
}


}}
