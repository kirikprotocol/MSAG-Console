// 
// File:   CommandApply.cc
// Author: igork
//
// Created on 27 Май 2004 г., 17:09
//

#include "SCAGCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "scag/scag.h"

namespace scag {
namespace admin {

using namespace smsc::util::xml;

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
