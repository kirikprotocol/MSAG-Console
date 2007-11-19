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

CentralPersServer::CentralPersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, const std::string& dbPath, const std::string& dbName, uint32_t indexGrowth, const char* regionsFileName):
        PersSocketServer(persHost_, persPort_, maxClientCount_, timeout_), logger(Logger::getInstance("cpersserv")), regions(NULL)
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

bool CentralPersServer::getProfileInfo(std::string& key, ProfileInfo& pi)
{
    return profileInfoStore.Get(AbntAddr(key.c_str()), pi);
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
            //isb.Empty();
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
  if (!getProfileInfo(get_profile.key, profile_info)) {
    smsc_log_debug(logger, "getProfileCmdHandler: profile key = \'%s\' not registered", get_profile.key.c_str());
    profile_info.owner = ctx.region_id;
    profileInfoStore.Insert(addr, profile_info);
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.serialize(osb);
    return;
  }
  smsc_log_debug(logger, "getProfileCmdHandler: profile key=\'%s\' registered owner=%d",
                  get_profile.key.c_str(), profile_info.owner);
  RegionInfo ri;
  if (!getRegionInfo(profile_info.owner, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d not found", profile_info.owner);
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.is_ok = 0;
    profile_resp.serialize(osb);
    return;
  }
  if (!ri.ctx) {
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d not available", profile_info.owner);
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.is_ok = 0;
    profile_resp.serialize(osb);
    return;
  }
  TransactionInfo tr_info;
  tr_info.owner = profile_info.owner;
  tr_info.candidate = ctx.region_id;
  transactions.Insert(AbntAddr(get_profile.key.c_str()), tr_info);

  //get_profile.serialize(ri.ctx->outbuf);
  sendCommand(get_profile, ri.ctx);
}

void CentralPersServer::profileRespCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "profileRespCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  ProfileRespCmd profile_resp(isb);
  AbntAddr addr(profile_resp.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "profileRespCmdHandler: Transcation with key=%s not found", profile_resp.key.c_str());
    DoneCmd done(0, profile_resp.key);
    done.serialize(osb);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->candidate, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "profileRespCmdHandler: region with id=%d not found", pti->candidate);
    DoneCmd done(0, profile_resp.key);
    done.serialize(osb);
    smsc_log_debug(logger, "profileRespCmdHandler: delete transaction key=\'%s\'",
                    profile_resp.key.c_str());
    transactions.Delete(addr);
    return;
  }
  if (!ri.ctx) {
    smsc_log_warn(logger, "profileRespCmdHandler: region with id=%d unavailable", pti->candidate);
    DoneCmd done(0, profile_resp.key);
    done.serialize(osb);
    smsc_log_debug(logger, "profileRespCmdHandler: delete transaction key=\'%s\'",
                    profile_resp.key.c_str());
    transactions.Delete(addr);
    return;
  }
  //profile_resp.serialize(ri.ctx->outbuf);
  sendCommand(profile_resp, ri.ctx);
}

void CentralPersServer::doneCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "doneCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done(isb);
  AbntAddr addr(done.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "doneCmdHandler: Transcation with key=%s not found", done.key.c_str());
    DoneRespCmd done_resp(0, done.key);
    done_resp.serialize(osb);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->owner, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "doneCmdHandler: region with id=%d not found", pti->candidate);
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
}

void CentralPersServer::doneRespCmdHandler(ConnectionContext& ctx) {
  smsc_log_debug(logger, "doneRespCmdHandler");
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done_resp(isb);
  AbntAddr addr(done_resp.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "doneRespCmdHandler: Transcation with key=%s not found", done_resp.key.c_str());
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->candidate, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "doneRespCmdHandler: region with id=%d not found", pti->candidate);
    smsc_log_debug(logger, "doneCmdHandler: delete transaction key=\'%s\'", done_resp.key.c_str());
    transactions.Delete(addr);
    return;
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

}}
