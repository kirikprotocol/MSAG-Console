//
// Class:   Abstract_CommandSmscInfo
// Author: Vitaly
//
// Created on 04.04.05
//

#include "Abstract_CommandSmscInfo.h"

#include "util/xml/utilFunctions.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

using namespace smsc::util::xml;
//
// Constructor
///
Abstract_CommandSmscInfo::Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document)
  : SCAGCommand(id)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("Command");
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      if (::strcmp("id", name) == 0)
        systemId = value.get();
      if (::strcmp("host", name) == 0)
        smscConfig.host = value.get();
      if (::strcmp("port", name) == 0)
        smscConfig.port = atoi(value.get());
      if (::strcmp("systemId", name) == 0) 
        smscConfig.sid = value.get();
      if (::strcmp("password", name) == 0)
        smscConfig.password = value.get();
      if (::strcmp("responseTimeout", name) == 0)
        smscConfig.smppTimeOut = atoi(value.get());
      if (::strcmp("altHost", name) == 0)
        altHost = value.get();
      if (::strcmp("altPort", name) == 0)
        altPort = atoi(value.get());
      if (::strcmp("uniqueMsgIdPrefix", name) == 0)
        uid = atoi(value.get());
    }
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

//
// Destructor
//
Abstract_CommandSmscInfo::~Abstract_CommandSmscInfo()
{
}

}
}
