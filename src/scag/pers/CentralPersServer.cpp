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
    ri = *pri;
    return true;
}

bool CentralPersServer::getProfileInfo(std::string& key, ProfileInfo& pi)
{
    return profileInfoStore.Get(AbntAddr(key.c_str()), pi);
}

bool CentralPersServer::processPacket(ConnectionContext& ctx)
{
    SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;

    try{
        uint8_t cmd = isb.ReadInt8();
        smsc_log_debug(logger, "cmd=%d", cmd);
        if(cmd == CentralPersCmd::LOGIN)
        {
            uint32_t id = isb.ReadInt32();
            std::string psw;
            isb.ReadString(psw);
            //LoginCmd login_cmd(isb);
            //uint32_t id = login_cmd.rp_id;
            //std::string psw = login_cmd.rp_psw;
            RegionInfo ri;
            smsc_log_debug(logger, "RP id=%d pswd=%s", id, psw.c_str());
            if (getRegionInfo(id, ri) && 
                (ctx.authed = !strncmp(ri.passwd.c_str(), psw.c_str(), 30))) {
              ctx.region_id = id;
              //ctx.wantRead = true;
              ri.ctx = &ctx;
              return true;
            } else {
              return false;
            }
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
              getProfileCmdHandler(ctx);
              smsc_log_debug(logger, "GET_PROFILE Profile received");
              break;
            case CentralPersCmd::PROFILE_RESP:
              profileRespCmdHandler(ctx);
              smsc_log_debug(logger, "PROFILE Resp received");
              break;
            case CentralPersCmd::DONE_RESP:
              doneRespCmdHandler(ctx);
              smsc_log_debug(logger, "DONE_RESP received");
              break;
            case CentralPersCmd::DONE:
              doneCmdHandler(ctx);
              smsc_log_debug(logger, "DONE received");
              break;
            case CentralPersCmd::CHECK_OWN:
              //checkOwnCmdHandler(ctx);
              smsc_log_debug(logger, "CHECK_OWN received");
              break;
        }
    }
    catch(...)
    {
    }
    return true;
}

void CentralPersServer::getProfileCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  GetProfileCmd get_profile;
  get_profile.deserialize(isb);
  ProfileInfo profile_info;
  AbntAddr addr(get_profile.key.c_str());
  if (!getProfileInfo(get_profile.key, profile_info)) {
    profile_info.owner = ctx.region_id;
    profileInfoStore.Insert(addr, profile_info);
    ProfileRespCmd profile_resp(get_profile.key);
    profile_resp.serialize(osb);
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(profile_info.owner, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d not found", profile_info.owner);
    return;
  }
  if (!ri.ctx) {
    smsc_log_warn(logger, "getProfileCmdHandler: region with id=%d not available", profile_info.owner);
    return;
  }
  TransactionInfo tr_info;
  tr_info.owner = profile_info.owner;
  tr_info.candidate = ctx.region_id;
  transactions.Insert(AbntAddr(get_profile.key.c_str()), tr_info);

  get_profile.serialize(ri.ctx->outbuf);
}

void CentralPersServer::profileRespCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  ProfileRespCmd profile_resp;
  profile_resp.deserialize(isb);
  AbntAddr addr(profile_resp.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "profileRespCmdHandler: Transcation with key=%s not found", profile_resp.key.c_str());
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->candidate, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "profileRespCmdHandler: region with id=%d not found", pti->candidate);
    return;
  }
  profile_resp.serialize(ri.ctx->outbuf);
}

void CentralPersServer::doneCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done;
  done.deserialize(isb);
  AbntAddr addr(done.key.c_str());
  TransactionInfo *pti;
  if(!(pti = transactions.GetPtr(addr))) {
    smsc_log_warn(logger, "doneCmdHandler: Transcation with key=%s not found", done.key.c_str());
    return;
  }
  RegionInfo ri;
  if (!getRegionInfo(pti->owner, ri)) {
    //region not found, but it can't be because in this case region wouldn't be authorised
    smsc_log_warn(logger, "doneCmdHandler: region with id=%d not found", pti->candidate);
    return;
  }
  if (done.is_ok) {
    ProfileInfo profile_info;
    profile_info.owner = pti->candidate;
    profileInfoStore.Insert(addr, profile_info);
  }
  done.serialize(ri.ctx->outbuf);
}

void CentralPersServer::doneRespCmdHandler(ConnectionContext& ctx) {
  SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;
  DoneCmd done_resp;
  done_resp.deserialize(isb);
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
    return;
  }
  transactions.Delete(addr);
  done_resp.serialize(ri.ctx->outbuf);
}


}}
