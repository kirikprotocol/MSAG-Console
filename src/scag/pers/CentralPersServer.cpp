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
                                     int timeout_, int transactTimeout_, const std::string& dbPath, const std::string& dbName,
                                     uint32_t indexGrowth, const char* regionsFileName):
 PersSocketServer(persHost_, persPort_, maxClientCount_, timeout_, transactTimeout_),
   logger(Logger::getInstance("cpersserv")), regions(NULL)
{
    reloadRegions(regionsFileName);
        
   	if(profileInfoAllocator.Init(dbPath + '/' + dbName, indexGrowth))
        throw SCAGException("Init ProfileInfoStore failed: dbPath=%s, dbName=%s", dbPath.c_str(), dbName.c_str());
        
   	profileInfoStore.SetAllocator(&profileInfoAllocator);
   	profileInfoStore.SetChangesObserver(&profileInfoAllocator);
    smsc_log_info(logger, "transactTimeout = %d", transactTimeout);
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
  smsc_log_debug(logger, "authorizeRegion: RP id=%d password=%s", id, psw.c_str());
  MutexGuard mt(regionsMapMutex);
  RegionInfo* pri = regions->GetPtr(id);
  if (!pri || !(ctx.authed = !strncmp(pri->passwd.c_str(), psw.c_str(), 30))) {
    return false;
  }
  if (pri->ctx != NULL) {
    smsc_log_warn(logger, "authorizeRegion: RP id=%d password=%s already connected", id, psw.c_str());
    return false;
  }
  ctx.region_id = id;
  pri->ctx = &ctx;
  return true;
}

void CentralPersServer::execCommand(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  uint8_t cmd = isb.ReadInt8();
  switch(cmd)
  {
      case CentralPersCmd::GET_PROFILE:
        smsc_log_debug(logger, "GET_PROFILE received");
        getProfileCmdHandler(ctx);
        break;
      case CentralPersCmd::PROFILE_RESP:
        smsc_log_debug(logger, "PROFILE_RESP received");
        profileRespCmdHandler(ctx);
        break;
      case CentralPersCmd::DONE_RESP:
        smsc_log_warn(logger, "DONE_RESP received");
        //doneRespCmdHandler(ctx);
        break;
      case CentralPersCmd::DONE:
        smsc_log_debug(logger, "DONE received");
        doneCmdHandler(ctx);
        break;
      case CentralPersCmd::CHECK_OWN:
        smsc_log_debug(logger, "CHECK_OWN received");
        checkOwnCmdHandler(ctx);
        break;
      case CentralPersCmd::PING:
        smsc_log_debug(logger, "PING received");
        pingCmdHandler(ctx);
        return;
      default:
        smsc_log_warn(logger, "UNKNOWN command received cmd_id=%d", cmd);
        return;
  }
  if (isb.getPos() < isb.GetSize()) {
    smsc_log_debug(logger, "execCommand: continue processing packet %p region id=%d inbuf pos=%d length=%d",
                    &ctx, ctx.region_id, isb.getPos(), isb.GetSize());
    execCommand(ctx);
  }
}

bool CentralPersServer::processPacket(ConnectionContext& ctx)
{
  try {
    smsc_log_debug(logger, "Start process packet %p from region id=%d",
                    &ctx, ctx.region_id);
    ctx.inbuf.SetPos(PACKET_LENGTH_SIZE);
    uint8_t cmd = ctx.inbuf.ReadInt8();
    if(cmd == CentralPersCmd::LOGIN)
    {
        return authorizeRegion(ctx);
    }
    else if(!ctx.authed) {
      smsc_log_warn(logger, "processPacket: RP not authorized");
      return false;
    }
    smsc_log_debug(logger, "processPacket: RP authorized id=%d", ctx.region_id);
    ctx.inbuf.SetPos(PACKET_LENGTH_SIZE);
    execCommand(ctx);
    smsc_log_debug(logger, "End process packet %p", &ctx);
  }
  catch(const SerialBufferOutOfBounds &e) {
    smsc_log_debug(logger, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                    ctx.inbuf.length(), ctx.inbuf.toString().c_str());
  }
  catch(const Exception& e) {
    smsc_log_debug(logger, "Exception: \'%s\'. Bad data in buffer received len=%d, data=%s",
                    e.what(), ctx.inbuf.length(), ctx.inbuf.toString().c_str());
  }
  catch(...) {
    smsc_log_debug(logger, "Exception. Bad data in buffer received len=%d, data=%s",
                    ctx.inbuf.length(), ctx.inbuf.toString().c_str());
  }
  return true;
}

void CentralPersServer::getProfileCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  GetProfileCmd get_profile(isb);
  ProfileInfo profile_info;
  AbntAddr addr(get_profile.key.c_str());
  TransactionInfo ti;
  if (transactions.Get(addr, ti)) {
    smsc_log_warn(logger, "getProfileCmdHandler: profile key=%s in process", get_profile.key.c_str());
    ProfileRespCmd profile_resp(get_profile.key);
    //profile_resp.is_ok = 0;
    profile_resp.setStatus(STATUS_LOCKED);
    sendResponse(profile_resp, ctx);
    return;
  }
  if (!getProfileInfo(addr, profile_info)) {
    smsc_log_debug(logger, "getProfileCmdHandler: profile key=%s not registered", get_profile.key.c_str());
    TransactionInfo tr_info(ownership::UNKNOWN, ctx.region_id);
    transactions.Insert(addr, tr_info);
    ProfileRespCmd profile_resp(get_profile.key);
    sendResponse(profile_resp, ctx);
    return;
  }
  if (profile_info.owner == ctx.region_id) {
    smsc_log_warn(logger, "getProfileCmdHandler: region id=%d is now profile key=%s owner",
                  ctx.region_id, get_profile.key.c_str());
    ProfileRespCmd profile_resp(get_profile.key);
    sendResponse(profile_resp, ctx);
    return;//????????
  }
  smsc_log_debug(logger, "getProfileCmdHandler: profile key=%s registered owner=%d",
                  get_profile.key.c_str(), profile_info.owner);
  RegionInfo ri;
  if (!getRegionInfo(profile_info.owner, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ? "unavailable" : "not found";
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d %s",
                   profile_info.owner, err_msg.c_str());
    ProfileRespCmd profile_resp(get_profile.key);
    sendResponse(profile_resp, ctx);
    smsc_log_debug(logger, "send %s to region id=%d", profile_resp.name.c_str(), ctx.region_id);
    return;
  }
  TransactionInfo tr_info(profile_info.owner, ctx.region_id);
  transactions.Insert(addr, tr_info);
  smsc_log_debug(logger, "getProfileCmdHandler: create transaction key=%s owner=%d candidate=%d",
                  get_profile.key.c_str(), tr_info.owner, tr_info.candidate);

  sendCommand(get_profile, ri.ctx);
}

void CentralPersServer::profileRespCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  ProfileRespCmd profile_resp(isb);
  AbntAddr addr(profile_resp.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "profileRespCmdHandler: Transcation with key=%s not found", profile_resp.key.c_str());
    DoneCmd done(0, profile_resp.key);
    sendResponse(done, ctx);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->candidate, ri) || !ri.ctx) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    string err_msg = !ri.ctx ? "not found" : "unavailable";
    smsc_log_warn(logger, "profileRespCmdHandler: region with id=%d %s, delete transaction key=%s",
                   pti->candidate, err_msg.c_str(), profile_resp.key.c_str());
    DoneCmd done(0, profile_resp.key);
    sendResponse(done, ctx);
    transactions.Delete(addr);
    return;
  }
  if (profile_resp.isOk()) {
    smsc_log_debug(logger, "profileRespCmdHandler: receive PROFILE_RESP=OK profile key=%s",
                    profile_resp.key.c_str());
    pti->wait_cmd = CentralPersCmd::DONE;
    pti->startTime = time(NULL);
  } else {
    string resp_status = profile_resp.getStatus() == STATUS_ERROR ? "ERROR" : "LOCKED";
    smsc_log_debug(logger, "profileRespCmdHandler: receive PROFILE_RESP=%s profile key=%s, delete transaction",
                   resp_status.c_str(), profile_resp.key.c_str());
    transactions.Delete(addr);
  }
  sendCommand(profile_resp, ri.ctx);
}

void CentralPersServer::doneCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done(isb);
  AbntAddr addr(done.key.c_str());
  TransactionInfo ti;
  if(!transactions.Get(addr, ti)) {
    smsc_log_warn(logger, "doneCmdHandler: Transcation with key=%s not found", done.key.c_str());
    DoneRespCmd done_resp(0, done.key);
    sendResponse(done_resp, ctx);
    return;
  }
  if (ti.owner == ownership::UNKNOWN) {
    if (done.is_ok) {
      ProfileInfo profile_info;
      profile_info.owner = ti.candidate;
      profileInfoStore.Insert(addr, profile_info);
      smsc_log_debug(logger, "doneCmdHandler: receive DONE=OK register profile key=%s, owner=%d",
                     done.key.c_str(), profile_info.owner);
      DoneRespCmd done_resp(1, done.key);
      sendResponse(done_resp, ctx);
    } else {
      smsc_log_debug(logger, "doneCmdHandler: receive DONE=ERROR profile key=%s not registered",
                      done.key.c_str());
    }
    transactions.Delete(addr);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(ti.owner, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "doneCmdHandler: region with id=%d not found", ti.candidate);
    DoneRespCmd done_resp(0, done.key);
    sendResponse(done_resp, ctx);
    transactions.Delete(addr);
    return;
  }
  if (done.is_ok) {
    smsc_log_debug(logger, "doneCmdHandler: receive DONE=OK, change profile key=%s owner old=%d new=%d",
                    done.key.c_str(), ti.owner, ti.candidate);
    ProfileInfo profile_info;
    profile_info.owner = ti.candidate;
    profileInfoStore.Set(addr, profile_info);
    DoneRespCmd done_resp(1, done.key);
    sendResponse(done_resp, ctx);
  } else {
    smsc_log_debug(logger, "doneCmdHandler: receive DONE=ERROR profile key=%s", done.key.c_str());
  }
  transactions.Delete(addr);
  sendCommand(done, ri.ctx);
}
/*
void CentralPersServer::doneRespCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "doneRespCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneRespCmd done_resp(isb);
  smsc_log_debug(logger, "doneRespCmdHandler: profile key = \'%s\'", done_resp.key.c_str());
  AbntAddr addr(done_resp.key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
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
    string err_msg = !ri.ctx ?  "unavailable" : "not found";;
    smsc_log_warn(logger, "doneRespCmdHandler: region with id=%d %s", owner, err_msg.c_str());
  }
  if (!done_resp.is_ok) {
    smsc_log_warn(logger, "doneRespCmdHandler: receive error DONE_RESP profile \'%s\'",
                  done_resp.key.c_str());
    ProfileInfo pi;
    pi.owner = ctx.region_id;
    profileInfoStore.Set(addr, pi);
  }
  transactions.Delete(addr);
  smsc_log_debug(logger, "doneRespCmdHandler: delete transaction key=\'%s\'", addr.toString().c_str());
  //done_resp.serialize(ri.ctx->outbuf);
  sendCommand(done_resp, ri.ctx);
}
*/
void CentralPersServer::checkOwnCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  CheckOwnCmd check_own(isb);
  smsc_log_debug(logger, "checkOwnCmdHandler: profile key=%s", check_own.key.c_str());
  ProfileInfo profile_info;
  AbntAddr addr(check_own.key.c_str());
  CheckOwnRespCmd check_own_resp(ownership::UNKNOWN, check_own.key);
  if (getProfileInfo(addr, profile_info)) {
    if (profile_info.owner == ctx.region_id) {
      smsc_log_debug(logger, "checkOwnCmdHandler: region id=%d is profile key=%s owner",
                     ctx.region_id, check_own.key.c_str());
      check_own_resp.result = ownership::OWNER;
    } else {
      smsc_log_debug(logger, "checkOwnCmdHandler: region id=%d is not profile key=%s owner",
                     ctx.region_id, check_own.key.c_str());
      check_own_resp.result = ownership::NOT_OWNER;
    }
  } else {
    smsc_log_warn(logger, "checkOwnCmdHandler: profile key=%s not registered", check_own.key.c_str());
  }
  sendResponse(check_own_resp, ctx);
}

void CentralPersServer::pingCmdHandler(ConnectionContext& ctx) {
  ctx.outbuf.SetPos(PACKET_LENGTH_SIZE);
  ctx.outbuf.WriteInt8(CentralPersCmd::PING_OK);
  ctx.outbuf.SetPos(0);
  ctx.outbuf.WriteInt32(ctx.outbuf.GetSize());
}

void CentralPersServer::sendCommand(const CPersCmd& cmd, ConnectionContext* ctx) {
  if (!ctx) {
    return;
  }
  smsc_log_debug(logger, "send %s to region id=%d", cmd.name.c_str(), ctx->region_id);
  cmd.serialize(ctx->outbuf);
  ctx->outbuf.SetPos(0);
  listener.addRW(ctx->socket);
}

void CentralPersServer::sendResponse(const CPersCmd& cmd, ConnectionContext& ctx) {
  cmd.serialize(ctx.outbuf);
  smsc_log_debug(logger, "send %s to region id=%d", cmd.name.c_str(), ctx.region_id);
}

void CentralPersServer::transactionTimeout(const AbntAddr& addr, const TransactionInfo& tr_info) {
  uint8_t wait_cmd(tr_info.wait_cmd);
  string key(addr.toString());
  switch (wait_cmd) {             
  case CentralPersCmd::PROFILE_RESP: {
    smsc_log_warn(logger, "PROFILE_RESP timeout, profile key=%s owner=%d candidate=%d",
                   key.c_str(), tr_info.owner, tr_info.candidate);
    ProfileRespCmd profile_resp(key);
    //profile_resp.is_ok = 0;
    profile_resp.setStatus(STATUS_ERROR);
    sendCommand(profile_resp, tr_info.candidate);
    DoneCmd done(0, key);
    sendCommand(done, tr_info.owner);
    break;
  }
  case CentralPersCmd::DONE: {
    smsc_log_warn(logger, "DONE timeout, profile key=%s owner=%d candidate=%d",
                   key.c_str(), tr_info.owner, tr_info.candidate);
    if (tr_info.owner != ownership::UNKNOWN) {
      DoneCmd done(0, key);
      sendCommand(done, tr_info.owner);
    }
      //DoneRespCmd done_resp(0, key);
      //sendCommand(done_resp, tr_info.candidate);
    //} else {
      DoneRespCmd done_resp(0, key);
      sendCommand(done_resp, tr_info.candidate);
      //CheckOwnRespCmd check_own_resp(ownership::NOT_OWNER, key);
      //sendCommand(check_own_resp, tr_info.candidate);
    //}
    break;
  }
  default:
    smsc_log_warn(logger, "UNKNWON command timeout profile key=%s", key.c_str());
    break;
  }
  transactions.Delete(addr);
}

void CentralPersServer::sendCommand(const CPersCmd &cmd, uint32_t region_id) {
  RegionInfo ri;
  if (!getRegionInfo(region_id, ri)) {
    smsc_log_error(logger, "sendCommand: region id=%d not found", region_id);
    return;
  }
  sendCommand(cmd, ri.ctx);
}

void CentralPersServer::checkTransactionsTimeouts() {
  if (isStopped()) {
    return;
  }
  smsc_log_debug(logger, "Checking transactions timeouts...");
  time_t time_bound = time(NULL) - transactTimeout;
  AbntAddr key;
  TransactionInfo tr_info;
  transactions.First();
  while (transactions.Next(key, tr_info)) {
    if (tr_info.startTime < time_bound) {
      transactionTimeout(key, tr_info);
    }
  }
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
