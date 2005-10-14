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
                              

namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;


SCAGCommand::SCAGCommand(Command::Id id) :Command(id),
    logger(smsc::logger::Logger::getInstance("admin.command"))
{
}

SCAGCommand::~SCAGCommand()
{
}

Response * SCAGCommand::CreateResponse(scag::Scag * ScagApp)
{
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
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
      GETSTRPARAM((char*)smppEntityInfo.password,      "password")
      GETINTPARAM(smppEntityInfo.timeOut,               "timeout")
      GETINTPARAM(smppEntityInfo.providerId,            "providerId")
            
      if (::strcmp("mode", name) == 0) {
        if (::strcmp("trx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        else if(::strcmp("tx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
        else if(::strcmp("rx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
        else
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;

        smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.timeOut);
      }

    }

    smppEntityInfo.type = scag::transport::smpp::etService;
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

Abstract_CommandSmeInfo::~Abstract_CommandSmeInfo()
{
}

//================================================================

Response * CommandAddSme::CreateResponse(scag::Scag * ScagApp)
{
  try {
      if(!ScagApp)
          Exception("Scag undefined");

      smsc_log_info(logger, "CommandAddSme is processing...");
      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->addSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandAddSme is processed ok");
      return new Response(Response::Ok, "CommandAddSme is processed ok");
  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to add new SME. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to add new SME. Unknown error");
      return new Response(Response::Error, "Failed to add new SME. Unknown error");
  }
}

//================================================================

using namespace xercesc;

CommandDeleteSme::CommandDeleteSme(const xercesc::DOMDocument * const document)
  : SCAGCommand((Command::Id)CommandIds::deleteSme)
{
    smsc_log_info(logger, "CommandDeleteSme got parameters:");

  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      if (::strcmp("systemId", name) == 0){
          smsc_log_info(logger, "systemId: %s", value.get());
        systemId = value.get();
      }
    }
  } catch (...) {
    smsc_log_warn(logger, "Failed to read parameters of Delete Sme command");
    throw AdminException("Some exception occured");
  }
}

Response * CommandDeleteSme::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandDeleteSme is processing...");
  try {
      if(!ScagApp)
          Exception("Scag undefined");
      ScagApp->getSmppManagerAdmin()->deleteSmppEntity(systemId.c_str());
  }catch(Exception& e){
      smsc_log_info(logger, "CommandDeleteSme exception, %s", e.what());
  }catch(...){
      smsc_log_info(logger, "CommandDeleteSme exception, Unknown exception."    );
  }
  smsc_log_info(logger, "CommandDeleteSme is processed ok");
  return new Response(Response::Ok, "none");
}

//================================================================

Response * CommandStatusSme::CreateResponse(scag::Scag * ScagApp)
{

    if(!ScagApp)
        Exception("Scag undefined");
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
  try {
      smsc_log_info(logger, "CommandUpdateSmeInfo is processing...");

      if(!ScagApp)
          Exception("Scag undefined");

      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->updateSmppEntity(getSmppEntityInfo());  

      smsc_log_info(logger, "CommandUpdateSmeInfo processed ok.");
      return new Response(Response::Ok, "CommandUpdateSmeInfo processed ok.");
  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to update sme. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to update sme. Unknown error.");
      return new Response(Response::Error, "Failed to update sme. Unknown error.");
  }
  return new Response(Response::Error, "Failed to update sme. Unknown error.");
}

//================================================================
//================ Smsc commands =================================

Abstract_CommandSmscInfo::Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document)
  : SCAGCommand(id)
{

  smsc_log_info(logger, "Abstract_CommandSmscInfo got parameters:");
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      if (::strcmp("systemId", name) == 0){
        strcpy(smppEntityInfo.systemId, value.get());
        smsc_log_info(logger, "systemId: %s", value.get());
      }
      if (::strcmp("password", name) == 0){
        strcpy(smppEntityInfo.password, value.get());
        smsc_log_info(logger, "password: %s", value.get());
      }

      if (::strcmp("timeout", name) == 0){
        smppEntityInfo.timeOut = atoi(value.get());
        smsc_log_info(logger, "timeout: %d", smppEntityInfo.timeOut);
      }

      if (::strcmp("providerId", name) == 0){
        smppEntityInfo.providerId = atoi(value.get());
        smsc_log_info(logger, "providerId: %d", smppEntityInfo.uid);
      }

      GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
      GETSTRPARAM((char*)smppEntityInfo.password,      "password")
      GETINTPARAM(smppEntityInfo.timeOut,               "timeout")
      GETINTPARAM(smppEntityInfo.providerId,            "providerId")
      GETINTPARAM(smppEntityInfo.uid,                   "uid")
            
      if (::strcmp("mode", name) == 0) {
        if (::strcmp("trx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        else if(::strcmp("tx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
        else if(::strcmp("rx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
        else
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.bindType);
      }

      GETSTRPARAM((char*)smppEntityInfo.host,       "host")
      GETINTPARAM(smppEntityInfo.port,              "port")
      GETSTRPARAM((char*)smppEntityInfo.altHost,    "altHost")
      GETINTPARAM(smppEntityInfo.altPort,           "altPort")
    }

    smppEntityInfo.type = scag::transport::smpp::etSmsc;
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

Abstract_CommandSmscInfo::~Abstract_CommandSmscInfo()
{
}

//================================================================

Response * CommandRemoveSmsc::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandUpdateSmsc is processing");
  try {
      if(!ScagApp)
          Exception("Scag undefined");

      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->updateSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandUpdateSmsc is processed ok");
      return new Response(Response::Ok, "none");
  }catch(Exception& e){
     char msg[1024];
     sprintf(msg, "Failed to update smsc. Details: %s", e.what());
     smsc_log_error(logger, msg);
     return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to update smsc. Unknown error.");
      return new Response(Response::Error, "Failed to update smsc. Unknown error.");
  }
}

//================================================================

Response * CommandAddSmsc::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandAddSmsc is processing...");
  try {

      if(!ScagApp)
          Exception("Scag undefined");

      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->updateSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandAddSmsc is processed ok");
      return new Response(Response::Ok, "none");
  }catch(exception& e){
      char msg[1024];
      sprintf(msg, "Failed to add smsc. Details: %s", e.what());

     smsc_log_warn(logger, msg);
     return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_warn(logger, "Failed to add smsc. Unknown error.");
      return new Response(Response::Error, "Failed to add smsc. Unknown error.");
  }
}

//================================================================

Response * CommandUpdateSmsc::CreateResponse(scag::Scag * ScagApp)
{ 
  smsc_log_info(logger, "CommandUnregSmsc is processing...");
  try {
      if(!ScagApp)
          Exception("Scag undefined");

      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->updateSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandUpdateSmsc is processed");
      return new Response(Response::Ok, "none");

  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to update smsc. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to update smsc. Unknown error");
      return new Response(Response::Error, "Failed to update smsc. Unknown error");
  }
  smsc_log_error(logger, "Failed to update smsc. Unknown error");
  return new Response(Response::Error, "Failed to update smsc. Unknown error");
  
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


Response * CommandLoadRoutes::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandLoadRoutes is processing");

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    result.appendValueToStringList("Routes configuration successfully loaded");

  try
  {
      ConfigManager& cfg = ConfigManager::Instance();
      cfg.reloadConfig(scag::config::ROUTE_CFG);

      /*RouteConfig cfg;
      if (cfg.load("conf/routes__.xml") == RouteConfig::fail)
          throw AdminException("Load routes config file failed.");

      vector<std::string> traceBuff;

      SmscApp->reloadTestRoutes(cfg);
      SmscApp->getTestRouterInstance()->enableTrace(true);
      SmscApp->getTestRouterInstance()->getTrace(traceBuff);*/

      // 0:   Message (Routes successfully loaded)
      // 1..: Trace (if any)

      smsc::admin::service::Variant result(smsc::admin::service::StringListType);
      result.appendValueToStringList("Routes configuration successfully loaded");

      //for (int i=0; i<traceBuff.size(); i++)
      //    result.appendValueToStringList(traceBuff[i].c_str());
  }
  catch (AdminException& aexc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", aexc.what());
      throw;
  }
  catch (ConfigException& cexc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", cexc.what());
      throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
  }
  catch (std::exception& exc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", exc.what());
      throw AdminException("Load routes failed. Cause: %s.", exc.what());
  }
  catch (...) {
      smsc_log_info(logger, "CommandLoadRoutes exception, unknown exception.");
      throw AdminException("Load routes failed. Cause is unknown.");
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



CommandTraceRoute::CommandTraceRoute(const xercesc::DOMDocument * doc)  
  : SCAGCommand((Command::Id)CommandIds::traceRoute)
{
  smsc_log_info(logger, "CommandStatusSme got parameters:");

  try {
    DOMElement *elem = doc->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));

    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      GETSTRPARAM_(srcAddr,     "srcAddress")
      GETSTRPARAM_(dstAddr,     "dstAddress")
      GETSTRPARAM_(srcSysId,    "srcSysId")

    }
  } catch (...) {
      smsc_log_warn(logger, "Failed to read parameters of Status Sme command");
    throw AdminException("Failed to read parameters of Status Sme command");
  }

}

CommandTraceRoute::~CommandTraceRoute()
{
  id = undefined;
}

Response * CommandTraceRoute::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandTraceRoute is processing...");
  const char* _srcAddr  = srcAddr.data();
  const char* _dstAddr  = dstAddr.data();
  const char* _srcSysId = srcSysId.data();

  smsc_log_info(logger, "---- Entered GetTraceResult()");
  smsc_log_info(logger, "---- Received command parameters: %s,%s,%s \n",srcAddr.data(),dstAddr.data(),srcSysId.data());


  try
  {
       
      smsc::smeman::SmeProxy* proxy = 0;
      smsc::router::RouteInfo info;
      bool found = false;
      info.enabling = true;

      //-1:   Dealiased destination
      // 0:   Message (Route found | Route found (disabled) | Route not found)
      // 1:   RouteInfo (if any)
      // 2..: Trace (if any)

      smsc::admin::service::Variant result(smsc::admin::service::StringListType);


      Address dealiased;
      char addrBuf[MAX_ADDRESS_VALUE_LENGTH+5];

      /*if(SmscApp->AliasToAddress(Address(_dstAddr),dealiased))
      {
        dealiasText="Address "+Address(_dstAddr).toString()+" was dealiased to "+dealiased.toString();
        dealiased.toString(addrBuf,sizeof(addrBuf));
        _dstAddr=addrBuf;
      }

      fprintf(stderr,"---- Passed dealiasing dstAddr");

      if (_srcSysId)
      {
          SmeIndex index = SmscApp->getSmeIndex(_srcSysId);
          if (index == -1)
              throw AdminException("Trace route failed. Sme for system id '%s' not found",_srcSysId);

          found = SmscApp->getTestRouterInstance()
                ->lookup(index, Address(_srcAddr), Address(_dstAddr), proxy, 0, &info);
      }
      else
      {
          found = SmscApp->getTestRouterInstance()->
              lookup(Address(_srcAddr), Address(_dstAddr), proxy, 0, &info);
      }

      fprintf(stderr,"---- Passed lookup");

      vector<std::string> traceBuff;
      SmscApp->getTestRouterInstance()->getTrace(traceBuff);

      if (!found)
      {
        if (info.enabling == false)
        {
          result.appendValueToStringList("Route found (disabled)");
          found = true;
        } else
        {
          result.appendValueToStringList("Route not found");
          result.appendValueToStringList("");
        }
      } else
      {
        result.appendValueToStringList("Route found");
      }

      if (found)
      {
          char routeText[2048];
          char srcAddressText[64]; char dstAddressText[64];
          info.source.getText(srcAddressText, sizeof(srcAddressText));
          info.dest  .getText(dstAddressText, sizeof(dstAddressText));

          std::auto_ptr<char> encRouteId(getEncodedString(info.routeId.c_str()));
          std::auto_ptr<char> encSrcAddressText(getEncodedString(srcAddressText));
          std::auto_ptr<char> encDstAddressText(getEncodedString(dstAddressText));
          std::auto_ptr<char> encSmeSystemId(getEncodedString(info.smeSystemId.c_str()));
          std::auto_ptr<char> encForwardTo(getEncodedString(info.forwardTo.c_str()));
          std::auto_ptr<char> encSrcSmeSystemId(getEncodedString(info.srcSmeSystemId.c_str()));

          sprintf(routeText, "route id:%s;source address:%s;destination address:%s;"
                             "sme system id:%s;source sme system id:%s;"
                             "priority:%u;service id:%d;delivery mode:%u;forward to:%s;"
                             "billing:%s;archiving:%s;enabling:%s;suppress delivery reports:%s",
                  encRouteId.get(), encSrcAddressText.get(), encDstAddressText.get(),
                  encSmeSystemId.get(), encSrcSmeSystemId.get(),
                  info.priority, info.serviceId, info.deliveryMode, encForwardTo.get(),
                  (info.billing) ? "yes":"no" , (info.archived) ? "yes":"no",
                  (info.enabling) ? "yes":"no", (info.suppressDeliveryReports) ? "yes":"no");

          result.appendValueToStringList(routeText);
      }

      if(dealiasText.length())
      {
        result.appendValueToStringList(dealiasText.c_str());
        result.appendValueToStringList("");
      }
      for (int i=0; i<traceBuff.size(); i++)
          result.appendValueToStringList(traceBuff[i].c_str());*/

      smsc_log_info(logger, "CommandTraceRoute is processed ok");
      return new Response(Response::Ok, result);
  }
  catch (AdminException& aexc) {
      char msg[1024];
      sprintf(msg, "Failed to trace route, details: %s", aexc.what());
      smsc_log_error(logger, msg);
      throw AdminException(msg);
  }
  catch (ConfigException& cexc) {
      char msg[1024];
      sprintf(msg, "Failed to trace route, details: %s", cexc.what());
      smsc_log_error(logger, msg);
      throw AdminException(msg);
  }
  catch (std::exception& exc) {
      char msg[1024];
      sprintf(msg, "Failed to trace route, details: %s", exc.what());
      smsc_log_error(logger, msg);
      throw AdminException(msg);
  }
  catch (...) {
      smsc_log_error(logger, "Failed to trace route, unknown error.");
      throw AdminException("Failed to trace route, unknown error.");
  }

  
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
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      GETINTPARAM(ruleId, "ruleId")

    }

  } catch (...) {
      smsc_log_warn(logger, "Failed to read parametrs during new add Rule, unknown exception");
    throw AdminException("Some exception occured");
  }
}

CommandAddRule::~CommandAddRule()
{
}

Response * CommandAddRule::CommandCreate(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandAddRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

      smsc_log_info(logger, "CommandAddRule is processed ok");
      return new Response(Response::Ok, "none");

  }catch(RuleEngineException& e){
      char desc[512];
      sprintf(desc, "Failed to add rule. RuleEngineException exception: %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());
      Variant res((const char *)desc);

      smsc_log_info(logger, desc);
      return new Response(Response::Error, res);
  }catch(Exception& e){

      char desc[512];
      sprintf(desc, "Failed to add new Rule. RuleEngine exception: %s", e.what());
      Variant res((const char *)desc);

      smsc_log_error(logger, desc);
      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Failed to add new Rule. Unknown error.");

      smsc_log_error(logger, "Failed to add new Rule. Unknown error.");
      return new Response(Response::Error, res);

  }

  smsc_log_error(logger, "Unknown error");
  return new Response(Response::Error, "Unknown error");
}

//================================================================

CommandRemoveRule::CommandRemoveRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{

  smsc_log_info(logger, "CommandRemoveRule got parameters:");

  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      GETINTPARAM(ruleId, "ruleId")

    }

  } catch (...) {
      smsc_log_warn(logger, "Failed to read parameters of remove rule command.");
    throw AdminException("Some exception occured");
  }
}

CommandRemoveRule::~CommandRemoveRule()
{
}

Response * CommandRemoveRule::CommandCreate(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandRemoveRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

      smsc_log_info(logger, "CommandRemoveRule is processed ok");
      return new Response(Response::Ok, "none");

  }catch(RuleEngineException& e){

      char desc[512];
      sprintf(desc, "Failed to remove rule. RuleEngineException exception: %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());
      Variant res((const char *)desc);

      smsc_log_info(logger, desc);
      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Failed to remove rule. Unknown error");
      smsc_log_error(logger, "Failed to remove rule. Unknown error");
      return new Response(Response::Error, res);

  }

  smsc_log_error(logger, "Unknown error");
  return new Response(Response::Error, "Unknown error");
}

//================================================================

CommandUpdateRule::CommandUpdateRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{
    smsc_log_info(logger, "CommandUpdateRule got parameters:");

  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
            
      GETINTPARAM(ruleId, "ruleId")

    }

  } catch (...) {
      smsc_log_warn(logger, "Failed to read parameters of Update Rule command.");
    throw AdminException("Failed to read parameters of Update Rule command.");
  }
}

CommandUpdateRule::~CommandUpdateRule()
{
}

Response * CommandUpdateRule::CommandCreate(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandUpdateRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

      smsc_log_info(logger, "CommandUpdateRule is processed ok.");
      return new Response(Response::Ok, "Failed to update rule. Unknown error.");

  }catch(RuleEngineException& e){

      char desc[512];
      sprintf(desc, "Failed to update rule. RuleEngineException exception: %s. Error in rule_%d.xml in line %d", e.what(), ruleId, e.getLineNumber());
      Variant res((const char *)desc);

      smsc_log_info(logger, desc);
      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Failed to update rule. Unknown error.");

      smsc_log_error(logger, "Failed to update rule. Unknown error.");
      return new Response(Response::Error, res);

  }

  smsc_log_error(logger, "Failed to update rule. Unknown error.");
  return new Response(Response::Error, "Failed to update rule. Unknown error.");
}

//================================================================
//================ Apply commands ================================

CommandApply::CommandApply(const xercesc::DOMDocument * doc)  
  : SCAGCommand((Command::Id)CommandIds::apply)
{
  smsc_log_info(logger, "CommandAddSme got parameters:");
  subj = CommandApply::unknown;
  try {
    DOMElement *elem = doc->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    if (list->getLength() > 0) {
      DOMElement *paramElem = (DOMElement*) list->item(0);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      if (::strcmp("subj", name) == 0) {
        std::auto_ptr<char> value(getNodeText(*paramElem));
        
        if (strcmp("config", value.get()) == 0)
          subj = CommandApply::config;
        else if (strcmp("routes", value.get()) == 0)
          subj = CommandApply::routes;
        else if (strcmp("providers", value.get()) == 0)
          subj = CommandApply::providers;
        else if (strcmp("smscs", value.get()) == 0)
          subj = CommandApply::smscs;
        else
          subj = CommandApply::unknown;

        smsc_log_info(logger, "subj: %s, %d", value.get(), subj);
          
      }
    }
  }
  catch (...) {
    smsc_log_warn(logger, "Failed to read parameters of Apply command");
    throw AdminException("Some exception occured");
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
      result.restart = true;
    case CommandApply::routes:
      result.reloadconfig = true;
      result.restart = true;
    case CommandApply::smscs:
      result.restart = true;
    case CommandApply::providers:
      result.restart = true;

  }
  return result;
}

}
}

