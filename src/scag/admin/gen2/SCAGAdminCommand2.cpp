//
// File:   SCAGAdminCommand.cpp
// Author: loomox
//
// Created on 24 march 2005
//

#include <vector>
#include "scag/admin/CommandIds.h"
#include "SCAGAdminCommand2.h"
#include "util/xml/utilFunctions.h"

#include <sms/sms.h>
#include <sms/sms_const.h>

#include "scag/transport/smpp/SmppTypes.h"
#include "util/xml/utilFunctions.h"
#include "admin/service/Variant.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/transport/smpp/base/SmppManagerAdmin2.h"

#include <sms/sms.h>
#include <sms/sms_const.h>
#include "util/Exception.hpp"
#include "util/config/ConfigException.h"
#include "admin/service/Type.h"

#include "scag/config/route/RouteStructures.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/transport/http/base/HttpProcessor.h"
#include "scag/transport/http/base/HttpRouter.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/counter/Manager.h"

#define CATCH_ADMINEXC(msg_)                                    \
      }catch(AdminException& e){                                \
        char msg[1024];                                         \
        snprintf(msg, sizeof(msg), msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg, responseEncoding);
#define CATCH_CFGEXC(msg_)                                      \
      }catch(smsc::util::config::ConfigException& e){                               \
        char msg[1024];                                         \
        snprintf(msg, sizeof(msg), msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg, responseEncoding);
#define CATCH_STDEXC(msg_)                                      \
      }catch(std::exception& e){                                \
        char msg[1024];                                         \
        snprintf(msg, sizeof(msg), msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg, responseEncoding);

namespace scag2 {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace scag::exceptions;
using namespace scag2::transport::smpp;



//================================================================
//================ Sme commands ==================================

using namespace smsc::util::xml;


void Abstract_CommandSmeInfo::init()
{

    smsc_log_info(logger, "Abstract_CommandSmeInfo got parameters:");
    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(systemId,      "systemId")
    END_SCAN_PARAMS
}


//================================================================


Response * CommandAddSme::CreateResponse(Scag * ScagApp)
{
    if (!ScagApp) throw Exception("Scag undefined");

    smsc_log_info(logger, "CommandAddSme is processing...");
    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if (!smppMan) throw Exception("SmppManager undefined");
    smsc_log_info(logger, "systemId: %s", systemId.c_str());

    SmppEntityInfo info;
    if(!smppMan->LoadEntityFromConfig(info,systemId.c_str(),etService))
    {
      throw Exception("Failed to load sme entity with systemId='%s'",systemId.c_str());
    }

    try {
        smppMan->addSmppEntity(info);
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to add new SME. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add new SME. Unknown exception");
        throw AdminException("Failed to add new SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandAddSme is processed ok");
    return new Response(Response::Ok, "CommandAddSme is processed ok", responseEncoding);
}

//================================================================

using namespace xercesc;

void CommandDeleteSme::init()
{
    smsc_log_info(logger, "Command(Delete/Disconnect)Sme got parameters:");

    disconn = 0;

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(systemId, "systemId")
    GETINTPARAM(disconn, "disconnect")
    END_SCAN_PARAMS

    if (systemId == "")
    {
        smsc_log_warn(logger, "Failed to read parameter 'systemId' of CommandDeleteSme.");
        throw AdminException("Failed to read parameter 'systemId' of CommandDeleteSme.");
    }
}

Response * CommandDeleteSme::CreateResponse(Scag * ScagApp)
{
    const char *n = disconn ? "Disconnect" : "Delete";

    smsc_log_info(logger, "Command%sSme is processing...", n);

    if (!ScagApp) throw Exception("Scag undefined");
    smsc_log_info(logger, "systemId: %s", systemId.c_str());

    try {
        if(disconn)
            ScagApp->getSmppManagerAdmin()->disconnectSmppEntity(systemId.c_str());
        else
            ScagApp->getSmppManagerAdmin()->deleteSmppEntity(systemId.c_str());
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to %s SME. Details: %s", n, e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to %s SME. Unknown exception", n);
        throw AdminException("Failed to %s SME. Unknown exception", n);
    }

    smsc_log_info(logger, "Command%sSme is processed ok", n);
    return new Response(Response::Ok, "none", responseEncoding);
}

//================================================================

Response * CommandStatusSme::CreateResponse(Scag * ScagApp)
{
    if (!ScagApp) throw Exception("Scag undefined");
    smsc_log_info(logger, "CommandStatusSme is processing...");

    Variant result(smsc::admin::service::StringListType);
  //((SmeManager*)(SmscApp->getSmeAdmin()))->statusSme(result);

    smsc_log_info(logger, "CommandStatusSme processed ok");
    return new Response(Response::Ok, result, responseEncoding);
}

//================================================================

Response * CommandUpdateSmeInfo::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandUpdateSmeInfo is processing...");

    if(!ScagApp) throw Exception("Scag undefined");
    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if(!smppMan) throw Exception("SmppManager undefined");

    SmppEntityInfo info;
    if(!smppMan->LoadEntityFromConfig(info,systemId.c_str(),etService))
    {
      throw Exception("Failed to load sme entity with systemId='%s'",systemId.c_str());
    }

    try {
        smppMan->updateSmppEntity(info);
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to update SME. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update SME. Unknown exception");
        throw AdminException("Failed to update SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmeInfo processed ok.");
    return new Response(Response::Ok, "CommandUpdateSmeInfo processed ok.", responseEncoding);
}


//================================================================
//================ Route commands ================================

using namespace smsc::sms;
using config::ConfigManager;



Response * CommandLoadSmppTraceRoutes::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandLoadSmppTraceRoutes is processing");

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    // result.appendValueToStringList("Routes configuration successfully loaded");

    if (!ScagApp) throw AdminException("ScagApp undefined");
    RouteConfig cfg;
    if (cfg.load("conf/smpp_routes__.xml") == RouteConfig::fail) throw AdminException("Load routes config file failed.");

    std::vector<std::string> traceBuff;

  try {

      SmppManager::Instance().reloadTestRoutes(cfg);
      SmppManager::Instance().getTestRouterInstance()->enableTrace(true);
      SmppManager::Instance().getTestRouterInstance()->getTrace(traceBuff);

      // 0:   Message (Routes successfully loaded)
      // 1..: Trace (if any)

      // smsc::admin::service::Variant result(smsc::admin::service::StringListType);
      result.appendValueToStringList("Routes configuration successfully loaded");

      smsc_log_info(logger, "traceBuff size: %d", traceBuff.size());

      for (unsigned i=0; i<traceBuff.size(); i++) {
          result.appendValueToStringList(traceBuff[i].c_str());
          smsc_log_info(logger, "traceBuff[%d]: %s", i, traceBuff[i].c_str());
      }

  CATCH_ADMINEXC("CommandLoadSmppTraceRoutes exception")
  CATCH_CFGEXC("CommandLoadSmppTraceRoutes exception")
  CATCH_STDEXC("CommandLoadSmppTraceRoutes exception")

  } catch (...) {
      smsc_log_warn(logger, "CommandLoadSmppTraceRoutes exception. Unknown exception");
      throw AdminException("CommandLoadSmppTraceRoutes exception. Unknown exception");
  }

  smsc_log_info(logger, "CommandLoadSmppTraceRoutes is processed ok");
  return new Response(Response::Ok, result, responseEncoding);
}


//================================================================

inline const size_t getEncodedStringSize(const char* const src)
{
    size_t count = 0;
    for (const char *p = src; *p != 0; p++)
        count += (*p == '#' || *p == ':' || *p == ';') ? 2:1;
    return count;
}
inline char* getEncodedString(const char* const src)
{
    char* result = new char[getEncodedStringSize(src)+1];
    char* d = result;
    for (const char *s = src; *s != 0; s++)
    {
        switch(*s)
        {
            case '#': *d++='#'; *d++='#'; break;
            case ':': *d++='#'; *d++='c'; break;
            case ';': *d++='#'; *d++='s'; break;
            default : *d++=*s;
        }
    }
    *d='\0';
    return result;
}



void CommandTraceSmppRoute::init()
{
    smsc_log_info(logger, "CommandTraceSmppRoute got parameters:");

    srcAddr = "";
    dstAddr = "";
    srcSysId = "";

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(srcAddr,     "srcAddress")
    GETSTRPARAM_(dstAddr,     "dstAddress")
    GETSTRPARAM_(srcSysId,    "srcSysId")
    END_SCAN_PARAMS

    std::string errorStr;

    if (srcAddr == "") errorStr = "Failed to read parameter 'srcAddress' of CommandTraceRoute";
    if (dstAddr == "") errorStr = "Failed to read parameter 'dstAddress' of CommandTraceRoute";
//    if (srcSysId == "") errorStr = "Failed to read parameter 'srcSysId' of CommandTraceRoute";

    if (errorStr.size() > 0)
    {
        smsc_log_warn(logger, errorStr.c_str());
        throw AdminException(errorStr.c_str());
    }
}

Response * CommandTraceSmppRoute::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandTraceSmppRoute is processing...");
    const char* _srcAddr  = srcAddr.c_str();
    const char* _dstAddr  = dstAddr.c_str();
    const char* _srcSysId = srcSysId.c_str();

    smsc_log_info(logger, "---- Entered GetTraceResult()");
    smsc_log_info(logger, "---- Received command parameters: %s,%s,%s \n",srcAddr.data(),dstAddr.data(),srcSysId.data());

    transport::smpp::router::RouteInfo info;
    bool found = false;
    info.enabled = true;

    //-1:   Dealiased destination
    // 0:   Message (Route found | Route found (disabled) | Route not found)
    // 1:   RouteInfo (if any)
    // 2..: Trace (if any)

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    Address dealiased;
    // char addrBuf[MAX_ADDRESS_VALUE_LENGTH+5];

    Address addr(_dstAddr);

    fprintf(stderr,"---- Passed dealiasing dstAddr");

    if (!ScagApp) throw Exception("Scag undefined");

    if (_srcSysId)
          found = SmppManager::Instance().getTestRouterInstance()->lookup(_srcSysId, Address(_srcAddr), Address(_dstAddr), info);
    else
          found = SmppManager::Instance().getTestRouterInstance()->lookup(Address(_srcAddr), Address(_dstAddr), info);

    fprintf(stderr,"---- Passed lookup");

    std::vector<std::string> traceBuff;
    SmppManager::Instance().getTestRouterInstance()->getTrace(traceBuff);

    if (!found) {
        if (info.enabled == false)
        {
          result.appendValueToStringList("Route found (disabled)");
          found = true;
        }
        else {
          result.appendValueToStringList("Route not found");
          result.appendValueToStringList("");
        }
    }
    else
    {
        result.appendValueToStringList("Route found");

        char routeText[2048];
        char srcAddressText[64]; char dstAddressText[64];
        info.source.getText(srcAddressText, sizeof(srcAddressText));
        info.dest  .getText(dstAddressText, sizeof(dstAddressText));

        std::auto_ptr<char> encRouteId(getEncodedString(info.routeId.c_str()));
        std::auto_ptr<char> encSrcAddressText(getEncodedString(srcAddressText));
        std::auto_ptr<char> encDstAddressText(getEncodedString(dstAddressText));
        std::auto_ptr<char> encSmeSystemId(getEncodedString(info.smeSystemId.c_str()));
        std::auto_ptr<char> encSrcSmeSystemId(getEncodedString(info.srcSmeSystemId.c_str()));

        snprintf(routeText, sizeof(routeText), 
                           "route id:%s;source address:%s;destination address:%s;"
                           "sme system id:%s;source sme system id:%s;archiving:no;"
                           "enabling:%s",
                encRouteId.get(), encSrcAddressText.get(), encDstAddressText.get(),
                encSmeSystemId.get(), encSrcSmeSystemId.get(),
                // (info.archived) ? "yes":"no",
                (info.enabled) ? "yes":"no");
        smsc_log_info(logger, "routeText: %s", routeText);

        result.appendValueToStringList(routeText);
      }

      for (unsigned i=0; i<traceBuff.size(); i++){
          result.appendValueToStringList(traceBuff[i].c_str());
          smsc_log_info(logger, "traceBuff[%d]: %s", i, traceBuff[i].c_str());
      }

      smsc_log_info(logger, "CommandTraceSmppRoute is processed ok");
      return new Response(Response::Ok, result, responseEncoding);

/*  CATCH_ADMINEXC("Failed to trace route")
  CATCH_CFGEXC("Failed to trace route")
  CATCH_STDEXC("Failed to trace route")
  CATCH("Failed to trace route.")
  }*/
}

//================================================================
//================ ApplyComfig command ===========================

Response * CommandApplyConfig::CreateResponse(Scag * SmscApp)
{
    smsc_log_info(logger, "CommandApplyConfig is processing...");

    ConfigManager & cfg = ConfigManager::Instance();

    cfg.reloadAllConfigs();

    smsc_log_info(logger, "CommandApplyConfig is processed ok");
    return new Response(Response::Ok, "none", responseEncoding);
}


//================================================================
//================ ApplySmppTraceRoutes command ================================


Response * CommandApplySmppRoutes::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandApplySmppRoutes is processing...");

    if (!ScagApp) throw Exception("Scag undefined");

    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if (!smppMan) throw Exception("SmppManager undefined");

    try {
        ConfigManager & cfg = ConfigManager::Instance();
        cfg.reloadConfig(config::ROUTE_CFG);
        smppMan->ReloadRoutes();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload routes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload routes. Unknown exception");
        throw AdminException("Failed to reload routes. Unknown exception");
    }

    smsc_log_info(logger, "CommandApplySmppRoutes is processed ok");
    return new Response(Response::Ok, "none", responseEncoding);
}

//================ ApplySmppTraceRoutes command ================================


Response * CommandApplyHttpRoutes::CreateResponse(Scag * SmscApp)
{
    smsc_log_info(logger, "CommandApplyHttpRoutes is processing...");

    try {
        transport::http::HttpProcessor::Instance().ReloadRoutes();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to apply HttpRoutes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to apply HttpRoutes. Unknown exception");
        throw AdminException("Failed to apply HttpRoutes. Unknown exception");
    }

    smsc_log_info(logger, "CommandApplyHttpRoutes processed ok.");
    return new Response(Response::Ok, "none", responseEncoding);
}


//-------------------- Bill Infrastructure commands ----------------------------
Response * CommandReloadOperators::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadOperators is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        bill::BillingManager& bill_mgr = bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadOperatorMap();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload operators. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload operators. Unknown exception");
        throw AdminException("Failed to reload operators. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadOperators processed ok.");
    return new Response(Response::Ok, "CommandReloadOperators processed ok.", responseEncoding);
}

Response * CommandReloadServices::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadServices is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        bill::BillingManager& bill_mgr = bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadProviderMap();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload services. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload services. Unknown exception");
        throw AdminException("Failed to reload services. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadServices processed ok.");
    return new Response(Response::Ok, "CommandReloadServices processed ok.", responseEncoding);
}

Response * CommandReloadTariffMatrix::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadTariffMatrix is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        bill::BillingManager& bill_mgr = bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadTariffMatrix();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload TariffMatrix. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload TariffMatrix. Unknown exception");
        throw AdminException("Failed to reload TariffMatrix. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadTariffMatrix processed ok.");
    return new Response(Response::Ok, "CommandReloadTariffMatrix processed ok.", responseEncoding);
}

Response * CommandLoadHttpTraceRoutes::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandLoadHttpTraceRoutes is processing...");
    Variant result(smsc::admin::service::StringListType);

    try {
        transport::http::HttpTraceRouter::Instance().ReloadRoutes();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload HttpTraceRoutes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        result.appendValueToStringList(msg);
        return new Response(Response::Error, result, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload HttpTraceRoutes. Unknown exception");
        throw AdminException("Failed to reload HttpTraceRoutes. Unknown exception");
    }

    smsc_log_info(logger, "CommandLoadHttpTraceRoutes processed ok.");
    result.appendValueToStringList("Trace routes successfully loaded.");
    return new Response(Response::Ok, result, responseEncoding);
}

/*
Response * CommandLoadHttpTraceRoutes::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandLoadHttpTraceRoutes is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        transport::http::HttpTraceRouter::Instance().ReloadRoutes();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Failed to reload HttpTraceRoutes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload HttpTraceRoutes. Unknown exception");
        throw AdminException("Failed to reload HttpTraceRoutes. Unknown exception");
    }

    smsc_log_info(logger, "CommandLoadHttpTraceRoutes processed ok.");
    return new Response(Response::Ok, "CommandLoadHttpTraceRoutes processed ok.");
} */


void CommandTraceHttpRoute::init()
{
    smsc_log_info(logger, "CommandTraceHttpRoute got parameters:");

    addr = "";
    site = "";
    path = "";
    port = 0;
    sid = 0;
    rid = 0;

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(addr,    "abonent")
    GETSTRPARAM_(site,    "site")
    GETSTRPARAM_(path,    "path")
    GETINTPARAM(port,    "port")
    GETINTPARAM(rid,    "rid")
    GETINTPARAM(sid,    "sid")
    END_SCAN_PARAMS

}

Response * CommandTraceHttpRoute::CreateResponse(Scag * ScagApp)
{
    bool b;
    smsc_log_info(logger, "CommandTraceHttpRoute is processing...");

    Variant result(smsc::admin::service::StringListType);

    try {
        if(port == 0) port = 80;

        std::vector<std::string> traceBuff;

        if(rid || sid)
            b = transport::http::HttpTraceRouter::Instance().getTraceRouteById(addr, path, rid, sid, traceBuff);
        else
            b = transport::http::HttpTraceRouter::Instance().getTraceRoute(addr, site, path, port, traceBuff);

        if(b)
            result.appendValueToStringList("Http route found");
        else
            result.appendValueToStringList("Http route not found");

        for (int i=0; i<traceBuff.size(); i++){
            result.appendValueToStringList(traceBuff[i].c_str());
            smsc_log_info(logger, "HttpTraceBuff[%d]: %s", i, traceBuff[i].c_str());
        }

    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Trace Http route failed. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to trace HttpRoute. Unknown exception");
        throw AdminException("Failed to trace HttpRoute. Unknown exception");
    }

    smsc_log_info(logger, "CommandTraceHttpRoute processed ok.");
    return new Response(Response::Ok, result, responseEncoding);
}


Response * CommandStoreLogConfig::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandStoreLogConfig is processing...");

    Variant result(smsc::admin::service::StringListType);

    try {
        Logger::Store();
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "CommandStoreLogConfig failed. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "CommandStoreLogConfig Failed. Unknown exception");
        throw AdminException("CommandStoreLogConfig Failed. Unknown exception");
    }

    smsc_log_info(logger, "CommandStoreLogConfig processed ok.");
    return new Response(Response::Ok, Variant(true), responseEncoding);
}

Response * CommandGetLogCategories::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandGetLogCategories is processing...");

    Variant result(smsc::admin::service::StringListType);

    try {
        std::auto_ptr<const Logger::LogLevels> cats(Logger::getLogLevels());
        char * k;
        Logger::LogLevel level;
        for (Logger::LogLevels::Iterator i = cats->getIterator(); i.Next(k, level); )
        {
            std::string tmp(k);
            tmp += ",";
            tmp += Logger::getLogLevel(level);
            smsc_log_debug(logger, "GetLogCategories [%s]=[%d, %s]", k, level, Logger::getLogLevel(level));
            result.appendValueToStringList(tmp.c_str());
        }
    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "CommandGetLogCategories failed. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "CommandGetLogCategories Failed. Unknown exception");
        throw AdminException("CommandGetLogCategories Failed. Unknown exception");
    }

    smsc_log_info(logger, "CommandGetLogCategories processed ok.");
    return new Response(Response::Ok, result, responseEncoding);
}

void CommandSetLogCategories::init()
{
    smsc_log_info(logger, "CommandSetLogCategories got parameters:");

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(cats,    "categories")
    END_SCAN_PARAMS
    smsc_log_info(logger, "CommandSetLogCategories cats=%s", cats.c_str());
}

Response * CommandSetLogCategories::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandSetLogCategories is processing...");

    const char *delim_pos, *p = cats.c_str();
    bool s = false;
    std::string cat, val;
    Logger::LogLevels levels;

    try{

    while((delim_pos = strchr(p, ',')))
    {
        if(!s)
        {
            int i = delim_pos - p - ((delim_pos > p && delim_pos[-1] == '\\') ? 1 : 0);
            cat.assign(p, i);
        }
        else
        {
            val.assign(p, delim_pos - p);
            levels[cat.c_str()] = Logger::getLogLevel(val.c_str());
//            smsc_log_debug(logger, "SetLogCategories [%s]=[%s]", cat.c_str(), val.c_str());
        }
        s = !s;
        p = delim_pos + 1;
    }

    if(s)
    {
        if(*p) levels[cat.c_str()] = Logger::getLogLevel(p);
    }
    else
    {
        smsc_log_info(logger, "CommandSetLogCategories processed failed. Bad format");
        return new Response(Response::Error, Variant(false), responseEncoding);
    }

    Logger::setLogLevels(levels);

    } catch(Exception& e) {
        char msg[1024];
        snprintf(msg, sizeof(msg), "CommandSetLogCategories failed. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "CommandSetLogCategories Failed. Unknown exception");
        throw AdminException("CommandSetLogCategories Failed. Unknown exception");
    }

    smsc_log_info(logger, "CommandSetLogCategories processed ok.");
    return new Response(Response::Ok, Variant(true), responseEncoding);
}


Response * CommandListSmppEntity::CreateResponse(Scag * ScagApp)
{
    if (!ScagApp) throw Exception("Scag undefined");

    smsc_log_info(logger, "%s is processing...", getCommandName());
    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if (!smppMan) throw Exception("SmppManager undefined");

    Variant result(smsc::admin::service::StringListType);
    std::auto_ptr<transport::smpp::SmppEntityAdminInfoList> entList(smppMan->getEntityAdminInfoList(getEntityType()));

    if (!entList.get()) throw Exception("%s error: SmppManager returns null", getCommandName());

    fillResultSet(result, *(entList.get()));

    smsc_log_info(logger, "%s is processed ok", getCommandName());
    return new Response(Response::Ok, result, responseEncoding);
}

void CommandMetaEntity::init()
{
  smsc_log_info(logger, "CommandMetaEntity got parameters:");
  BEGIN_SCAN_PARAMS
  GETSTRPARAM_(systemId,    "systemId")
  END_SCAN_PARAMS
}

Response * CommandAddMetaEntity::CreateResponse(Scag *ScagApp)
{
  transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
  if(!smppMan)
  {
    throw Exception("SmppManager not defined");
  }
  MetaEntityInfo mei;
  if(!smppMan->LoadMetaEntityFromConfig(mei,systemId.c_str()))
  {
    throw Exception("Failed to load metaentity from config:'%s'",systemId.c_str());
  }

  smppMan->addMetaEntity(mei);
  return new Response(Response::Ok,"Meta entity added", responseEncoding);
}

Response * CommandUpdateMetaEntity::CreateResponse(Scag *ScagApp)
{
  transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
  if(!smppMan)
  {
    throw Exception("SmppManager not defined");
  }
  MetaEntityInfo mei;
  if(!smppMan->LoadMetaEntityFromConfig(mei,systemId.c_str()))
  {
    throw Exception("Failed to load metaentity from config:'%s'",systemId.c_str());
  }
  smppMan->updateMetaEntity(mei);
  return new Response(Response::Ok,"Meta entity updated", responseEncoding);
}


void CommandDeleteMetaEntity::init()
{
  smsc_log_info(logger, "CommandDeleteMetaEntity got parameters:");
  BEGIN_SCAN_PARAMS
  GETSTRPARAM_(systemId,    "systemId")
  END_SCAN_PARAMS
}

Response * CommandDeleteMetaEntity::CreateResponse(Scag *ScagApp)
{
  transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
  if(!smppMan)
  {
    throw Exception("SmppManager not defined");
  }
  smppMan->deleteMetaEntity(systemId.c_str());
  return new Response(Response::Ok,"Meta entity deleted", responseEncoding);
}


void CommandMetaEndpoint::init()
{
  smsc_log_info(logger, "CommandMetaEndpoint got parameters:");
  BEGIN_SCAN_PARAMS
  GETSTRPARAM_(metaId,    "metaId")
  GETSTRPARAM_(sysId,     "sysId")
  END_SCAN_PARAMS
}

Response * CommandAddMetaEndpoint::CreateResponse(Scag *ScagApp)
{
  transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
  if(!smppMan)
  {
    throw Exception("SmppManager not defined");
  }
  smppMan->addMetaEndPoint(metaId.c_str(),sysId.c_str());
  return new Response(Response::Ok,"MetaEndpoint added", responseEncoding);
}

Response * CommandRemoveMetaEndpoint::CreateResponse(Scag *ScagApp)
{
  transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
  if(!smppMan)
  {
    throw Exception("SmppManager not defined");
  }
  smppMan->removeMetaEndPoint(metaId.c_str(),sysId.c_str());
  return new Response(Response::Ok,"MetaEndpoint removed", responseEncoding);
}


//================ Sme commands ==================================

void CommandReplaceCounter::init()
{

    smsc_log_info(logger, "CommandReplaceCounterActions got parameters:");
    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(id,"id")
    END_SCAN_PARAMS
}


Response* CommandReplaceCounter::CreateResponse( Scag* )
{
    // if (!app) throw Exception("scag undefined");
    smsc_log_info(logger,"CommandReplaceCounter is processing...");
    counter::Manager& mgr = counter::Manager::getInstance();
    const char* what;
    if ( getId() == CommandIds::replaceCounterActions ) {
        what = "actions replaced";
        mgr.reloadObserver( id.c_str() );
    } else if ( getId() == CommandIds::replaceCounterTemplate ) {
        what = "template replaced";
        mgr.reloadTemplate( id.c_str() );
    } else {
        throw Exception("unknown counter replace command id %d",getId());
    }
    return new Response(Response::Ok,what, responseEncoding);
}

}
}
