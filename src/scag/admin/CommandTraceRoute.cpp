// 
// File:   CommandTraceRoute.cpp
// Author: loomox
//
// Created on 24 march 2005
//

#include "CommandTraceRoute.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "smeman/smeproxy.h"
#include <util/config/route/RouteConfig.h>

#include <router/route_types.h>
#include <sms/sms.h>
#include <sms/sms_const.h>



namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::util::config::route;


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

      if (::strcmp("srcAddress", name) == 0){
        srcAddr = value.get();
        smsc_log_info(logger, "srcAddress: %s", value.get());
      }
      if (::strcmp("dstAddress", name) == 0){
        dstAddr = value.get();
        smsc_log_info(logger, "dstAddress: %s", value.get());
      }
      if (::strcmp("srcSysId", name) == 0){ 
        srcSysId = value.get();
        smsc_log_info(logger, "srcSysId: %s", value.get());
      }

    }
  } catch (...) {
      smsc_log_info(logger, "CommandTraceRoute exception, unknown exception");
    throw AdminException("Some exception occured");
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
      smsc_log_info(logger, "CommandTraceRoute exception, %s", aexc.what());
      throw;
  }
  catch (ConfigException& cexc) {
      smsc_log_info(logger, "CommandTraceRoute exception, %s", cexc.what());
      throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
  }
  catch (std::exception& exc) {
      smsc_log_info(logger, "CommandTraceRoute exception, %s", exc.what());
      throw AdminException("Trace route failed. Cause: %s.", exc.what());
  }
  catch (...) {
      smsc_log_info(logger, "CommandTraceRoute exception, unknown exception");
      throw AdminException("Trace route failed. Cause is unknown.");
  }

  
}

}
}
