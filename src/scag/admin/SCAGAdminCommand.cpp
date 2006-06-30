// 
// File:   SCAGAdminCommand.cpp
// Author: loomox
//
// Created on 24 march 2005 
//

#include "SCAGAdminCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"

#include <sms/sms.h>
#include <sms/sms_const.h>

#include "scag/transport/smpp/SmppTypes.h"
#include "util/xml/utilFunctions.h"
#include "CommandIds.h"

#include "admin/service/Variant.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

#include <sms/sms.h>
#include <sms/sms_const.h>
#include "util/Exception.hpp"
#include "admin/service/Type.h"

#include "scag/config/route/RouteStructures.h"

#include "scag/bill/BillingManager.h"
#include "scag/transport/http/HttpProcessor.h"
#include "scag/transport/http/HttpRouter.h"

#define CATCH_ADMINEXC(msg_)                                    \
      }catch(AdminException& e){                                \
        char msg[1024];                                         \
        sprintf(msg, msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg);
#define CATCH_CFGEXC(msg_)                                      \
      }catch(ConfigException& e){                               \
        char msg[1024];                                         \
        sprintf(msg, msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg);
#define CATCH_STDEXC(msg_)                                      \
      }catch(std::exception& e){                                \
        char msg[1024];                                         \
        sprintf(msg, msg_ " Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg);

namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace scag::exceptions;



//================================================================
//================ Sme commands ==================================

using namespace smsc::util::xml;


void Abstract_CommandSmeInfo::init()
{

    smsc_log_info(logger, "Abstract_CommandSmeInfo got parameters:");

    smppEntityInfo.systemId = "";
    smppEntityInfo.password = "";
    smppEntityInfo.timeOut = -1;
    //smppEntityInfo.providerId = -1;

    BEGIN_SCAN_PARAMS
    GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
    GETSTRPARAM((char*)smppEntityInfo.password,      "password")
    GETINTPARAM(smppEntityInfo.timeOut,              "timeout")
    //GETINTPARAM(smppEntityInfo.providerId,           "providerId")           

    if (::strcmp("mode", name) == 0) 
    {
        if (::strcmp("trx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        else if(::strcmp("tx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
        else if(::strcmp("rx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
        else smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;

        smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.timeOut);
    }
    END_SCAN_PARAMS

    smppEntityInfo.type = scag::transport::smpp::etService;

    std::string errorStr;

    if (smppEntityInfo.systemId == "") errorStr = "Failed to tead Sme parameter 'systemId'";
    //if (smppEntityInfo.password == "") errorStr = "Failed to tead Sme parameter 'password'";
    if (smppEntityInfo.timeOut == -1) errorStr = "Failed to tead Sme parameter 'timeout'";

    if (errorStr.size() > 0) 
    {
        smsc_log_warn(logger, errorStr.c_str());
        throw AdminException(errorStr.c_str());
    }
}


//================================================================


Response * CommandAddSme::CreateResponse(scag::Scag * ScagApp)
{
    if (!ScagApp) throw Exception("Scag undefined");

    smsc_log_info(logger, "CommandAddSme is processing...");
    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if (!smppMan) throw Exception("SmppManager undefined");
    smsc_log_info(logger, "systemId: %s", getSmppEntityInfo().systemId.c_str());

    try {
        smppMan->addSmppEntity(getSmppEntityInfo());
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to add new SME. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add new SME. Unknown exception");        
        throw AdminException("Failed to add new SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandAddSme is processed ok");
    return new Response(Response::Ok, "CommandAddSme is processed ok");
}

//================================================================

using namespace xercesc;

void CommandDeleteSme::init()
{
    smsc_log_info(logger, "CommandDeleteSme got parameters:");

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(systemId, "systemId")
    END_SCAN_PARAMS

    if (systemId == "") 
    {
        smsc_log_warn(logger, "Failed to read parameter 'systemId' of CommandDeleteSme.");
        throw AdminException("Failed to read parameter 'systemId' of CommandDeleteSme.");
    }
}

Response * CommandDeleteSme::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandDeleteSme is processing...");

    if (!ScagApp) throw Exception("Scag undefined");
    smsc_log_info(logger, "systemId: %s", systemId.c_str());

    try {
        ScagApp->getSmppManagerAdmin()->deleteSmppEntity(systemId.c_str());
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to delete SME. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to delete SME. Unknown exception");        
        throw AdminException("Failed to delete SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandDeleteSme is processed ok");
    return new Response(Response::Ok, "none");
}

//================================================================

Response * CommandStatusSme::CreateResponse(scag::Scag * ScagApp)
{
    if (!ScagApp) throw Exception("Scag undefined");
    smsc_log_info(logger, "CommandStatusSme is processing...");

    Variant result(smsc::admin::service::StringListType);
  //((SmeManager*)(SmscApp->getSmeAdmin()))->statusSme(result);

    smsc_log_info(logger, "CommandStatusSme processed ok");
    return new Response(Response::Ok, result);
}

//================================================================

Response * CommandUpdateSmeInfo::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandUpdateSmeInfo is processing...");

    if(!ScagApp) throw Exception("Scag undefined");
    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->updateSmppEntity(getSmppEntityInfo());  
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to update SME. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update SME. Unknown exception");        
        throw AdminException("Failed to update SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmeInfo processed ok.");
    return new Response(Response::Ok, "CommandUpdateSmeInfo processed ok.");
}


//================================================================
//================ Route commands ================================

using namespace smsc::sms;
using scag::config::ConfigManager;



Response * CommandLoadSmppTraceRoutes::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandLoadSmppTraceRoutes is processing");

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    result.appendValueToStringList("Routes configuration successfully loaded");

    if (!ScagApp) throw AdminException("ScagApp undefined");
    RouteConfig cfg;
    if (cfg.load("conf/smpp_routes__.xml") == RouteConfig::fail) throw AdminException("Load routes config file failed.");

    vector<std::string> traceBuff;

  try {

      ScagApp->reloadTestRoutes(cfg);
      ScagApp->getTestRouterInstance()->enableTrace(true);
      ScagApp->getTestRouterInstance()->getTrace(traceBuff);

      // 0:   Message (Routes successfully loaded)
      // 1..: Trace (if any)

      smsc::admin::service::Variant result(smsc::admin::service::StringListType);
      result.appendValueToStringList("Routes configuration successfully loaded");

      smsc_log_info(logger, "traceBuff size: %d", traceBuff.size());

      for (int i=0; i<traceBuff.size(); i++) {
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
  return new Response(Response::Ok, result);
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

Response * CommandTraceSmppRoute::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandTraceSmppRoute is processing...");
    const char* _srcAddr  = srcAddr.c_str();
    const char* _dstAddr  = dstAddr.c_str();
    const char* _srcSysId = srcSysId.c_str();

    smsc_log_info(logger, "---- Entered GetTraceResult()");
    smsc_log_info(logger, "---- Received command parameters: %s,%s,%s \n",srcAddr.data(),dstAddr.data(),srcSysId.data());

    scag::transport::smpp::router::RouteInfo info;
    bool found = false;
    info.enabled = true;

    //-1:   Dealiased destination
    // 0:   Message (Route found | Route found (disabled) | Route not found)
    // 1:   RouteInfo (if any)
    // 2..: Trace (if any)

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    Address dealiased;
    char addrBuf[MAX_ADDRESS_VALUE_LENGTH+5];

    Address addr(_dstAddr);

    fprintf(stderr,"---- Passed dealiasing dstAddr");

    if (!ScagApp) throw Exception("Scag undefined");

    if (_srcSysId) 
          found = ScagApp->getTestRouterInstance()->lookup(_srcSysId, Address(_srcAddr), Address(_dstAddr), info);
    else 
          found = ScagApp->getTestRouterInstance()->lookup(Address(_srcAddr), Address(_dstAddr), info);

    fprintf(stderr,"---- Passed lookup");

    vector<std::string> traceBuff;
    ScagApp->getTestRouterInstance()->getTrace(traceBuff);

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

        sprintf(routeText, "route id:%s;source address:%s;destination address:%s;"
                           "sme system id:%s;source sme system id:%s;archiving:%s;"
                           "enabling:%s",
                encRouteId.get(), encSrcAddressText.get(), encDstAddressText.get(),
                encSmeSystemId.get(), encSrcSmeSystemId.get(),
                (info.archived) ? "yes":"no",
                (info.enabled) ? "yes":"no");
        smsc_log_info(logger, "routeText: %s", routeText);

        result.appendValueToStringList(routeText);
      }

      for (int i=0; i<traceBuff.size(); i++){
          result.appendValueToStringList(traceBuff[i].c_str());
          smsc_log_info(logger, "traceBuff[%d]: %s", i, traceBuff[i].c_str());
      }

      smsc_log_info(logger, "CommandTraceSmppRoute is processed ok");
      return new Response(Response::Ok, result);

/*  CATCH_ADMINEXC("Failed to trace route")
  CATCH_CFGEXC("Failed to trace route")
  CATCH_STDEXC("Failed to trace route")
  CATCH("Failed to trace route.")
  }*/
}

//================================================================
//================ ApplySmppTraceRoutes command ================================


Response * CommandApplySmppRoutes::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandApplySmppRoutes is processing...");

    ConfigManager & cfg = ConfigManager::Instance();

    cfg.reloadConfig(scag::config::ROUTE_CFG);

    smsc_log_info(logger, "CommandApplySmppRoutes is processed ok");
    return new Response(Response::Ok, "none");
}

//-------------------- Bill Infrastructure commands ----------------------------
Response * CommandReloadOperators::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadOperators is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        scag::bill::BillingManager& bill_mgr = scag::bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadOperatorMap();
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to reload operators. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload operators. Unknown exception");        
        throw AdminException("Failed to reload operators. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadOperators processed ok.");
    return new Response(Response::Ok, "CommandReloadOperators processed ok.");
}

Response * CommandReloadServices::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadServices is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        scag::bill::BillingManager& bill_mgr = scag::bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadProviderMap();
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to reload services. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload services. Unknown exception");        
        throw AdminException("Failed to reload services. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadServices processed ok.");
    return new Response(Response::Ok, "CommandReloadServices processed ok.");
}

Response * CommandReloadTariffMatrix::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadTariffMatrix is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        scag::bill::BillingManager& bill_mgr = scag::bill::BillingManager::Instance();
        bill_mgr.getInfrastructure().ReloadTariffMatrix();
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to reload TariffMatrix. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload TariffMatrix. Unknown exception");        
        throw AdminException("Failed to reload TariffMatrix. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadTariffMatrix processed ok.");
    return new Response(Response::Ok, "CommandReloadTariffMatrix processed ok.");
}

Response * CommandReloadHttpRoutes::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadHttpRoutes is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        scag::transport::http::HttpProcessor::Instance().ReloadRoutes();
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to reload HttpRoutes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload HttpRoutes. Unknown exception");        
        throw AdminException("Failed to reload HttpRoutes. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadHttpRoutes processed ok.");
    return new Response(Response::Ok, "CommandReloadHttpRoutes processed ok.");
}

Response * CommandReloadHttpTraceRoutes::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandReloadHttpTraceRoutes is processing...");

//    if(!ScagApp) throw Exception("Scag undefined");

    try {
        scag::transport::http::HttpTraceRouter::Instance().ReloadRoutes();
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to reload HttpTraceRoutes. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to reload HttpTraceRoutes. Unknown exception");        
        throw AdminException("Failed to reload HttpTraceRoutes. Unknown exception");
    }

    smsc_log_info(logger, "CommandReloadHttpTraceRoutes processed ok.");
    return new Response(Response::Ok, "CommandReloadHttpTraceRoutes processed ok.");
}

void CommandTraceHttpRoute::init()
{
    smsc_log_info(logger, "CommandTraceHttpRoute got parameters:");

    addr = "";
    site = "";
    path = "";
    port = 0;

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(addr,    "abonent")
    GETSTRPARAM_(site,    "site")
    GETSTRPARAM_(path,    "path")
    GETINTPARAM(port,    "port")
    END_SCAN_PARAMS

    std::string errorStr;

    if (addr == "") errorStr = "Failed to read parameter 'abonent' of CommandTraceHttpRoute";
    if (site == "") errorStr = "Failed to read parameter 'site' of CommandTraceHttpRoute";

    if (errorStr.size() > 0) 
    {
        smsc_log_warn(logger, errorStr.c_str());
        throw AdminException(errorStr.c_str());
    }
}

Response * CommandTraceHttpRoute::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandTraceHttpRoute is processing...");

    std::string result;

    try {
        if(port == 0)
            port = 80;
        result = scag::transport::http::HttpTraceRouter::Instance().getTraceRoute(addr, site, path, port);
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Trace Http route failed. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to trace HttpRoute. Unknown exception");        
        throw AdminException("Failed to trace HttpRoute. Unknown exception");
    }

    smsc_log_info(logger, "CommandTraceHttpRoute processed ok.");
    return new Response(Response::Ok, result.c_str());
}

}
}

