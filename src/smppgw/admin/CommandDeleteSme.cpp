//
// File:   CommandDeleteSme.cc
// Author: igork
//
// Created on 31 ������ 2004 �., 17:55
//

#include "CommandDeleteSme.h"

#include "util/xml/utilFunctions.h"
#include "CommandIds.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace xercesc;
using namespace smsc::util::xml;
//
// Constructor
///
CommandDeleteSme::CommandDeleteSme(const xercesc::DOMDocument * const document)
  : Command((Command::Id)CommandIds::deleteSme)
{
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      if (::strcmp("id", name) == 0)
        systemid = value.get();
    }
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

}
}
}
