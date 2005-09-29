//
// File:   CommandDeleteSme.cc
// Author: igork
//
// Created on 31 á×ÇÕÓÔ 2004 Ç., 17:55
//

#include "CommandDeleteSme.h"

#include "util/xml/utilFunctions.h"
#include "CommandIds.h"
#include "scag/scag.h"

namespace scag {
namespace admin {

using namespace xercesc;
using namespace smsc::util::xml;
//
// Constructor
///
CommandDeleteSme::CommandDeleteSme(const xercesc::DOMDocument * const document)
  : SCAGCommand((Command::Id)CommandIds::deleteSme)
{
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      if (::strcmp("systemId", name) == 0)
        systemid = value.get();
    }
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

Response * CommandDeleteSme::CreateResponse(scag::Scag * SmscApp)
{
  //SmscApp->getSmeAdmin()->deleteSme(getSmeSystemId());
  return new Response(Response::Ok, "none");
}

}
}
