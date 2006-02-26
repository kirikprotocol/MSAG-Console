// 
// File:   SCAGCommand.cpp
// Author: loomox
//
// Created on 24 march 2005 
//

#include "SCAGCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"

#include <sms/sms.h>
#include <sms/sms_const.h>

#include "scag/transport/smpp/SmppTypes.h"
#include "util/xml/utilFunctions.h"
#include "CommandIds.h"

#include "scag/re/RuleEngine.h"
#include "admin/service/Variant.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

#include <sms/sms.h>
#include <sms/sms_const.h>
#include "util/Exception.hpp"
#include "admin/service/Type.h"
#include "scag/exc/SCAGExceptions.h"

#include "scag/config/route/RouteStructures.h"

#define GETSTRPARAM(param, name_)                           \
    if (::strcmp(name_, name) == 0){                        \
        strcpy(param, value.get());                         \
        smsc_log_info(logger, name_ ": %s", param);         \
    }                                                   

#define GETSTRPARAM_(param, name_)                          \
    if (::strcmp(name_, name) == 0){                        \
        param = value.get();                                \
        smsc_log_info(logger, name_ ": %s", param.c_str()); \
    }                                                   

#define GETINTPARAM(param, name_)                           \
    if (::strcmp(name_, name) == 0){                        \
        param = atoi(value.get());                          \
        smsc_log_info(logger, name_ ": %d", param);         \
    }


#define BEGIN_SCAN_PARAMS                                               \
    DOMElement *elem = document->getDocumentElement();                  \
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));    \
    for (int i=0; i<list->getLength(); i++) {                           \
      DOMElement *paramElem = (DOMElement*) list->item(i);              \
      XmlStr name(paramElem->getAttribute(XmlStr("name")));             \
      std::auto_ptr<char> value(getNodeText(*paramElem));
      
#define END_SCAN_PARAMS }


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


SCAGCommand::SCAGCommand(Command::Id id) :Command(id),
    logger(smsc::logger::Logger::getInstance("admin.command"))
{
}

SCAGCommand::~SCAGCommand()
{
}

Response * SCAGCommand::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "SCAGCommand::CreateResponse has processed");
  return new Response(Response::Ok, "none");
}

Actions::CommandActions SCAGCommand::GetActions()
{
  Actions::CommandActions result;
  return result;
}

//================================================================
//================ Sme commands ==================================

using namespace smsc::util::xml;

Abstract_CommandSmeInfo::Abstract_CommandSmeInfo(const Command::Id id, const xercesc::DOMDocument * const document)
  : SCAGCommand(id)
{
    smsc_log_info(logger, "Abstract_CommandSmeInfo got parameters:");

    try {
  
        BEGIN_SCAN_PARAMS
        GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
        GETSTRPARAM((char*)smppEntityInfo.password,      "password")
        GETINTPARAM(smppEntityInfo.timeOut,              "timeout")
        GETINTPARAM(smppEntityInfo.providerId,           "providerId")           

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
    } catch (...) {
        smsc_log_warn(logger, "Failed to tead Sme parameters. Unknown exception");        
    }
}

Abstract_CommandSmeInfo::~Abstract_CommandSmeInfo()
{
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
        sprintf(msg, "Failed to add new SME. Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
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

CommandDeleteSme::CommandDeleteSme(const xercesc::DOMDocument * const document)
  : SCAGCommand((Command::Id)CommandIds::deleteSme)
{
    smsc_log_info(logger, "CommandDeleteSme got parameters:");

    try {

        BEGIN_SCAN_PARAMS
        GETSTRPARAM_(systemId, "systemId")
        END_SCAN_PARAMS

    } catch (...) 
    {
        smsc_log_warn(logger, "Failed to read parameters of Delete Sme command. Unknown exception");        
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
        sprintf(msg, "Failed to delete SME. Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
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

CommandUpdateSmeInfo::CommandUpdateSmeInfo(const xercesc::DOMDocument * const document)
  : Abstract_CommandSmeInfo((Command::Id)CommandIds::updateSmeInfo, document)
{
  smsc_log_info(logger, "UpdateSmeInfo command");
}

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
        sprintf(msg, "Failed to update SME. Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update SME. Unknown exception");        
        throw AdminException("Failed to update SME. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmeInfo processed ok.");
    return new Response(Response::Ok, "CommandUpdateSmeInfo processed ok.");
}

//================================================================
//================ Smsc commands =================================

Abstract_CommandSmscInfo::Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document)
  : SCAGCommand(id)
{

    smsc_log_info(logger, "Abstract_CommandSmscInfo got parameters:");
    try {

        BEGIN_SCAN_PARAMS

        GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
        GETSTRPARAM((char*)smppEntityInfo.password,      "password")
        GETSTRPARAM((char*)smppEntityInfo.bindSystemId,  "bindSystemId")
        GETSTRPARAM((char*)smppEntityInfo.bindPassword,  "bindPassword")
        GETINTPARAM(smppEntityInfo.timeOut,               "timeout")
        GETINTPARAM(smppEntityInfo.providerId,            "providerId")
        GETINTPARAM(smppEntityInfo.uid,                   "uid")
            
        if (::strcmp("mode", name) == 0) 
        {
            if (::strcmp("trx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
            else if(::strcmp("tx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
            else if(::strcmp("rx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
            else smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
            smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.bindType);
        }

        GETSTRPARAM((char*)smppEntityInfo.host,       "host")
        GETINTPARAM(smppEntityInfo.port,              "port")
        GETSTRPARAM((char*)smppEntityInfo.altHost,    "altHost")
        GETINTPARAM(smppEntityInfo.altPort,           "altPort")

        END_SCAN_PARAMS

        smppEntityInfo.type = scag::transport::smpp::etSmsc;
    } catch (...) {
      smsc_log_error(logger, "Some exception occured");
    }
}

Abstract_CommandSmscInfo::~Abstract_CommandSmscInfo()
{
}

//================================================================

CommandDeleteSmsc::CommandDeleteSmsc(const xercesc::DOMDocument * const document)
  : SCAGCommand((Command::Id)CommandIds::deleteSmsc)
{
    smsc_log_info(logger, "CommandDeleteSme got parameters:");

    try {

        BEGIN_SCAN_PARAMS
        GETSTRPARAM_(systemId, "systemId")
        END_SCAN_PARAMS

    } catch (...) {
      smsc_log_error(logger, "Failed to read parameters of Delete Sme command.");
    }
}

Response * CommandDeleteSmsc::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandDeleteSmsc is processing");
    if(!ScagApp) throw Exception("Scag undefined");
    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->deleteSmppEntity(systemId.c_str());
    }
    catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to delete smsc. Details: %s", e.what());            \
        smsc_log_error(logger, msg);                            \
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to delete smsc. Unknown exception");        
        throw AdminException("Failed to delete smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandDeleteSmsc is processed ok");
    return new Response(Response::Ok, "CommandDeleteSmsc is processed ok");
}

//================================================================

Response * CommandAddSmsc::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandAddSmsc is processing...");
    if(!ScagApp) throw Exception("Scag undefined");

    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->addSmppEntity(getSmppEntityInfo());
    }
    catch(exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to add smsc. Details: %s", e.what());            
        smsc_log_error(logger, msg);                            
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add smsc. Unknown exception");        
        throw AdminException("Failed to add smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandAddSmsc is processed ok");
    return new Response(Response::Ok, "none");
}

//================================================================

Response * CommandUpdateSmsc::CreateResponse(scag::Scag * ScagApp)
{ 
    smsc_log_info(logger, "CommandUnregSmsc is processing...");

    if (!ScagApp) throw Exception("Scag undefined");

    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if (!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->updateSmppEntity(getSmppEntityInfo());
    }
    catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to update smsc. Details: %s", e.what());            
        smsc_log_error(logger, msg);                            
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update smsc. Unknown exception");        
        throw AdminException("Failed to update smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmsc is processed");
    return new Response(Response::Ok, "none");
}

//================================================================
//================ Route commands ================================

using namespace smsc::sms;
using scag::config::ConfigManager;



CommandLoadRoutes::CommandLoadRoutes(const xercesc::DOMDocument * doc)
  : SCAGCommand((Command::Id)CommandIds::loadRoutes)
{
  smsc_log_debug(logger, "LoadRoutes command");
}


Response * CommandLoadRoutes::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandLoadRoutes is processing");

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    result.appendValueToStringList("Routes configuration successfully loaded");

    if (!ScagApp) throw AdminException("ScagApp undefined");
    RouteConfig cfg;
    if (cfg.load("conf/routes__.xml") == RouteConfig::fail) throw AdminException("Load routes config file failed.");

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

  CATCH_ADMINEXC("CommandLoadRoutes exception")
  CATCH_CFGEXC("CommandLoadRoutes exception")
  CATCH_STDEXC("CommandLoadRoutes exception")

  } catch (...) {
      smsc_log_warn(logger, "CommandLoadRoutes exception. Unknown exception");
      throw AdminException("CommandLoadRoutes exception. Unknown exception");
  }

  smsc_log_info(logger, "CommandLoadRoutes is processed ok");
  return new Response(Response::Ok, result);
}

CommandLoadRoutes::~CommandLoadRoutes()
{
  id = undefined;
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



CommandTraceRoute::CommandTraceRoute(const xercesc::DOMDocument * document)  
  : SCAGCommand((Command::Id)CommandIds::traceRoute)
{
    smsc_log_info(logger, "CommandStatusSme got parameters:");

    try {

        BEGIN_SCAN_PARAMS
        GETSTRPARAM_(srcAddr,     "srcAddress")
        GETSTRPARAM_(dstAddr,     "dstAddress")
        GETSTRPARAM_(srcSysId,    "srcSysId")
        END_SCAN_PARAMS
    } catch (...) {
        smsc_log_warn(logger, "Failed to read parameters of Status Sme command. Unknown exception");
    }
}

CommandTraceRoute::~CommandTraceRoute()
{
  id = undefined;
}

Response * CommandTraceRoute::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandTraceRoute is processing...");
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

      smsc_log_info(logger, "CommandTraceRoute is processed ok");
      return new Response(Response::Ok, result);

/*  CATCH_ADMINEXC("Failed to trace route")
  CATCH_CFGEXC("Failed to trace route")
  CATCH_STDEXC("Failed to trace route")
  CATCH("Failed to trace route.")
  }*/
}

//================================================================
//================ Rule commands =================================

using smsc::admin::service::Variant;
using scag::exceptions::RuleEngineException;

CommandAddRule::CommandAddRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::addRule),
      ruleId( -1 )
{
    smsc_log_info(logger, "CommandAddRule got parameters:");

    try {

        BEGIN_SCAN_PARAMS
        GETINTPARAM(ruleId, "ruleId")
        END_SCAN_PARAMS
    } catch (...)
    {
        smsc_log_error(logger,"Failed to read parametrs during new add Rule.");
    }
}

CommandAddRule::~CommandAddRule()
{
}

Response * CommandAddRule::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandAddRule is processing...");
    try {
        scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
        re.updateRule(ruleId);

    } catch(RuleEngineException& e){                             
        char desc[512];                                         
        sprintf(desc, "Failed to add rule. RuleEngineException exception: %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());
        Variant res(smsc::admin::service::StringListType); 

        res.appendValueToStringList(desc);
        smsc_log_info(logger, desc);                            
        char buff[32];
        sprintf(buff,"%d",e.getLineNumber());			
        res.appendValueToStringList(buff);			
        return new Response(Response::Ok, res);
    } catch(Exception& e) {                                     
        char msg[1024];                                         
        sprintf(msg, "Failed to add new Rule. Details: %s", e.what());            
        smsc_log_error(logger, msg);                            
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add new Rule. Unknown exception");
        throw AdminException("Failed to add new Rule. Unknown exception");
    }

    Variant okRes("");
    smsc_log_info(logger, "CommandAddRule is processed ok");
    return new Response(Response::Ok, okRes);
}

//================================================================

CommandRemoveRule::CommandRemoveRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{

    smsc_log_info(logger, "CommandRemoveRule got parameters:");

    try {

        BEGIN_SCAN_PARAMS
        GETINTPARAM(ruleId, "ruleId")
        END_SCAN_PARAMS

    } catch (...) {
        smsc_log_warn(logger, "Failed to read parameters of remove rule command. Unknown exception");
    }
}

CommandRemoveRule::~CommandRemoveRule()
{
}

Response * CommandRemoveRule::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandRemoveRule is processing...");
    try {
        scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
        re.removeRule(ruleId);

    } catch(RuleEngineException& e) {                             
        char desc[512];                                         
        sprintf(desc, "Failed to remove rule. %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());   

        Variant res(smsc::admin::service::StringListType);

        res.appendValueToStringList(desc);
        smsc_log_info(logger, desc);                            
        char buff[32];						
        sprintf(buff,"%d",e.getLineNumber());			
        res.appendValueToStringList(buff);			
        return new Response(Response::Ok, res);

    } catch (SCAGException& e) {
        char desc[512];                                         
        sprintf(desc, "Failed to remove rule. %s.", e.what(), ruleId);   

        Variant res((const char *)desc);                        
        smsc_log_info(logger, desc);                            
        return new Response(Response::Error, res);
    } catch (...) {
        smsc_log_warn(logger, "Failed to remove rule. Unknown exception");
        throw AdminException("Failed to remove rule. Unknown exception");
    }

    Variant okRes("");
    smsc_log_info(logger, "CommandRemoveRule is processed ok");
    return new Response(Response::Ok, okRes);
}

//================================================================

CommandUpdateRule::CommandUpdateRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{
    smsc_log_info(logger, "CommandUpdateRule got parameters:");

    try {

        BEGIN_SCAN_PARAMS           
        GETINTPARAM(ruleId, "ruleId")
        END_SCAN_PARAMS
    } catch (...) {
        smsc_log_warn(logger, "Failed to read parameters of Update Rule command. Unknown exception");
    }

}

CommandUpdateRule::~CommandUpdateRule()
{
}

Response * CommandUpdateRule::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandUpdateRule is processing...");
    try {
        scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
        re.updateRule(ruleId);

    } catch(RuleEngineException& e) {
        char desc[512];                                         
        sprintf(desc, "Failed to update rule. %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());   
        Variant res(smsc::admin::service::StringListType);
        res.appendValueToStringList(desc);

        smsc_log_info(logger, desc);                            
        char buff[32];						
        sprintf(buff,"%d",e.getLineNumber());			
        res.appendValueToStringList(buff);
        
        Response * response = new Response(Response::Ok, res);
        //smsc_log_info(logger, response->getText()); 

        return response;

    } catch (SCAGException& e) {
        char desc[512];                                         
        sprintf(desc, "Failed to update rule. %s.", e.what(), ruleId);   

        Variant res((const char *)desc);                        
        smsc_log_info(logger, desc);                            
        return new Response(Response::Error, res);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update rule. Unknown exception");
        throw AdminException("Failed to update rule. Unknown exception");
    }

    Variant okRes("");

    smsc_log_info(logger, "CommandUpdateRule is processed ok.");
    return new Response(Response::Ok, okRes);
}

//================================================================
//================ Apply commands ================================

CommandApply::CommandApply(const xercesc::DOMDocument * document)  
  : SCAGCommand((Command::Id)CommandIds::apply)
{
    smsc_log_info(logger, "CommandApply got parameters:");
    subj = CommandApply::unknown;

    try {

        BEGIN_SCAN_PARAMS
        if (::strcmp("subj", name) == 0) 
        {
            if (strcmp("config", value.get()) == 0) subj = CommandApply::config;
            else if (strcmp("routes", value.get()) == 0) subj = CommandApply::routes;
            else if (strcmp("providers", value.get()) == 0) subj = CommandApply::providers;
            else if (strcmp("smscs", value.get()) == 0) subj = CommandApply::smscs;
            else subj = CommandApply::unknown;

            smsc_log_info(logger, "subj: %s, %d", value.get(), subj);
          
        }
        END_SCAN_PARAMS

    } catch (...) {
        smsc_log_warn(logger, "Failed to read parameters of Apply command. Unknown exception");
    }

}

CommandApply::~CommandApply()
{
    id = undefined;
    subj = CommandApply::unknown;
}

Response * CommandApply::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandApply is processing...");
    smsc_log_info(logger, "CommandApply is processed ok");
    return new Response(Response::Ok, "none");
}

Actions::CommandActions CommandApply::GetActions()
{
    Actions::CommandActions result;

    switch (subj)
    {
    case CommandApply::config:
        result.reloadconfig = true;
        break;
    case CommandApply::routes:
        result.reloadroutes = true;
        break;
    case CommandApply::smscs:
        result.reloadsmscs = true;
        break;
    case CommandApply::providers:
        result.reloadconfig = true;
        break;

    }
    return result;
}

}
}

