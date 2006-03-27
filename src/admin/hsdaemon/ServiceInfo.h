#ifndef __SMSC_ADMIN_HSDAEMON_SERVICEINFO_H__
#define __SMSC_ADMIN_HSDAEMON_SERVICEINFO_H__

#include <string>
#include <util/xml/utilFunctions.h>

namespace smsc{
namespace admin{
namespace hsdaemon{


struct ServiceInfo{
  enum ServiceType { failover, standalone };
  ServiceInfo():autoStart(false),serviceType(failover),autostartDelay(0){}
  void InitFromXml(const xercesc::DOMDocument *doc)
  {
    using namespace smsc::util::xml;
    using namespace xercesc;

    try
    {
      DOMElement *elem = doc->getDocumentElement();
      DOMNodeList *list = elem->getElementsByTagName(XmlStr("service"));
      if (list->getLength() > 0)
      {
        DOMElement *serviceElem = (DOMElement*) list->item(0);
        id=XmlStr(serviceElem->getAttribute(XmlStr("id"))).c_str();
        args=XmlStr(serviceElem->getAttribute(XmlStr("args"))).c_str();
        autoStart = strcmp("true", XmlStr(serviceElem->getAttribute(XmlStr("autostart")))) == 0;
        XmlStr type(serviceElem->getAttribute(XmlStr("serviceType")));
        if(strcmp(type,"failover")==0)
        {
          serviceType=smsc::admin::hsdaemon::ServiceInfo::failover;
        }else if(strcmp(type,"standalone")==0)
        {
          serviceType=smsc::admin::hsdaemon::ServiceInfo::standalone;
        }else
        {
          throw smsc::util::Exception("Unknown service type:%s",(const char*)type);
        }
        XmlStr host(serviceElem->getAttribute(XmlStr("hostName")));
        hostName=host.c_str();

        XmlStr autodelay(serviceElem->getAttribute(XmlStr("autostartDelay")));
        if(strlen(autodelay.c_str())>0)
        {
          autostartDelay=atoi(autodelay.c_str());
        }
        XmlStr prefNode(serviceElem->getAttribute(XmlStr("preferedNode")));
        preferedNode=prefNode.c_str();
      }
    }
    catch (std::exception& e)
    {
      throw AdminException("Exception occured:%s",e.what());
    }
    catch (...)
    {
      throw AdminException("Exception occured: unknown");
    }
  }

  std::string id;
  std::string args;
  std::string hostName;
  std::string preferedNode;
  bool autoStart;
  ServiceType serviceType;
  int autostartDelay;
};

}//hsdaemon
}//admin
}//smsc

#endif
