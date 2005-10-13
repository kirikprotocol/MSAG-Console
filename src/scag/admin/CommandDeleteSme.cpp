//
// File:   CommandDeleteSme.cc
// Author: igork
//
// Created on 31 á×ÇÕÓÔ 2004 Ç., 17:55
//

#include "SCAGCommand.h"

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

}
}
