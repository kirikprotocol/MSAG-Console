// 
// File:   CommandApply.cc
// Author: igork
//
// Created on 27 ��� 2004 �., 17:09
//

#include "CommandApply.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;

CommandApply::CommandApply(const xercesc::DOMDocument * doc)
  : Command((Command::Id)CommandIds::apply)
{
  smsc_log_debug(logger, "Apply command");
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
        else
          subj = CommandApply::unknown;
      }
    }
  }
  catch (...) {
    throw AdminException("Some exception occured");
  }
}

CommandApply::~CommandApply()
{
  id = undefined;
}

CommandApply::subjects CommandApply::getSubject()
{
  return subj;
}

}
}
}
