//
// File:   Abstract_CommandSmeInfo.cc
// Author: igork
//
// Created on 31 á×ÇÕÓÔ 2004 Ç., 18:07
//

#include "Abstract_CommandSmeInfo.h"

#include "smeman/smeinfo.h"
#include "util/xml/utilFunctions.h"
#include "CommandIds.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::smppgw::admin;
//
// Constructor
///
Abstract_CommandSmeInfo::Abstract_CommandSmeInfo(const Command::Id id, const xercesc::DOMDocument * const document)
  : SmppGwCommand(id)
{
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      if (::strcmp("id", name) == 0)
        smeInfo.systemId = value.get();
      if (::strcmp("priority", name) == 0)
        smeInfo.priority = atoi(value.get());
      if (::strcmp("type", name) == 0) {
        //skip it, sme can be SMPP type only
      }
      if (::strcmp("typeOfNumber", name) == 0)
        smeInfo.typeOfNumber = atoi(value.get());
      if (::strcmp("numberingPlan", name) == 0)
        smeInfo.numberingPlan = atoi(value.get());
      if (::strcmp("interfaceVersion", name) == 0)
        smeInfo.interfaceVersion = atoi(value.get());
      if (::strcmp("systemType", name) == 0)
        smeInfo.systemType = value.get();
      if (::strcmp("password", name) == 0)
        smeInfo.password = value.get();
      if (::strcmp("addrRange", name) == 0)
        smeInfo.rangeOfAddress = value.get();
      if (::strcmp("timeout", name) == 0)
        smeInfo.timeout = atoi(value.get());
      if (::strcmp("receiptSchemeName", name) == 0)
        smeInfo.receiptSchemeName = value.get();
      if (::strcmp("disabled", name) == 0)
        smeInfo.disabled = ::strcmp("true", value.get()) == 0;
      if (::strcmp("mode", name) == 0) {
        if (::strcmp("trx", value.get()) == 0)
          smeInfo.bindMode = smsc::smeman::smeTRX;
        else if(::strcmp("tx", value.get()) == 0)
          smeInfo.bindMode = smsc::smeman::smeTX;
        else if(::strcmp("rx", value.get()) == 0)
          smeInfo.bindMode = smsc::smeman::smeRX;
        else
          smeInfo.bindMode = smsc::smeman::smeTRX;
      }
      if (::strcmp("proclimit", name) == 0)
        smeInfo.proclimit = atoi(value.get());
      if (::strcmp("smsc", name) == 0) {
        //smeInfo.smsc = ::strcmp("true", value.get()) == 0;
      }
      if (::strcmp("providerId", name) == 0)
        smeInfo.providerId = atoi(value.get());
    }
  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

//
// Destructor
//
Abstract_CommandSmeInfo::~Abstract_CommandSmeInfo()
{
}

}
}
}
