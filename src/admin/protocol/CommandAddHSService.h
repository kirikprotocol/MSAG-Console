#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

#include "admin/hsdaemon/Service.h"
#include <admin/AdminException.h>
#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>
#include <admin/daemon/Service.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::AdminException;
using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;

typedef std::pair<int, char *> CmdArgument;

class CommandAddHSService : public CommandService
{
public:
  CommandAddHSService(const DOMDocument *doc) throw (AdminException)
    : CommandService(add_hsservice, doc)
  {
    smsc_log_debug(logger, "Add service command");
    try
    {
      DOMElement *elem = doc->getDocumentElement();
      DOMNodeList *list = elem->getElementsByTagName(XmlStr("service"));
      if (list->getLength() > 0)
      {
        DOMElement *serviceElem = (DOMElement*) list->item(0);
        serviceId.reset(XmlStr(serviceElem->getAttribute(XmlStr("id"))).c_release());
        args.reset(XmlStr(serviceElem->getAttribute(XmlStr("args"))).c_release());
        autostart = strcmp("true", XmlStr(serviceElem->getAttribute(XmlStr("autostart")))) == 0;
        XmlStr type(serviceElem->getAttribute(XmlStr("serviceType")));
        if(strcmp(type,"failover")==0)
        {
          svcType=smsc::admin::hsdaemon::Service::failover;
        }else if(strcmp(type,"standalone")==0)
        {
          svcType=smsc::admin::hsdaemon::Service::standalone;
        }else
        {
          throw smsc::util::Exception("Unknown service type:%s",(const char*)type);
        }
        XmlStr host(serviceElem->getAttribute(XmlStr("hostName")));
        hostName=host.c_str();
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

  /*const char * const getServiceName() const throw()
  {
    return serviceName.get();
  }*/

  const char * const getArgs() const throw ()
  {
    return args.get();
  }

  const bool isAutostart() const throw ()
  {
    return autostart;
  }

  smsc::admin::hsdaemon::Service::service_type getServiceType()const
  {
    return svcType;
  }

  const char* getHostName()const
  {
    return hostName.c_str();
  }

protected:
  std::auto_ptr<char> args;
  std::string hostName;
  bool autostart;
  smsc::admin::hsdaemon::Service::service_type svcType;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
