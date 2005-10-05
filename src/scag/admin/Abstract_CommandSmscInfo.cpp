//
// Class:   Abstract_CommandSmscInfo
// Author: Vitaly
//
// Created on 04.04.05
//

#include "Abstract_CommandSmscInfo.h"
#include "scag/transport/smpp/SmppTypes.h"

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

  smsc_log_info(logger, "Abstract_CommandSmscInfo got parameters:");
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));

      if (::strcmp("systemId", name) == 0){
        strcpy(smppEntityInfo.systemId, value.get());
        smsc_log_info(logger, "systemId: %s", value.get());
      }
      if (::strcmp("password", name) == 0){
        strcpy(smppEntityInfo.password, value.get());
        smsc_log_info(logger, "password: %s", value.get());
      }

      if (::strcmp("timeout", name) == 0){
        smppEntityInfo.timeOut = atoi(value.get());
        smsc_log_info(logger, "timeout: %d", smppEntityInfo.timeOut);
      }
            
      if (::strcmp("mode", name) == 0) {
        if (::strcmp("trx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        else if(::strcmp("tx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
        else if(::strcmp("rx", value.get()) == 0)
          smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
        else
          smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.bindType);
      }

      if (::strcmp("host", name) == 0){
        strcpy(smppEntityInfo.host, value.get());
        smsc_log_info(logger, "host: %s", value.get());
      }
      if (::strcmp("port", name) == 0){
        smppEntityInfo.port = atoi(value.get());
        smsc_log_info(logger, "port: %d", smppEntityInfo.port);
      }
      
      if (::strcmp("altHost", name) == 0){
        strcpy(smppEntityInfo.altHost, value.get());
        smsc_log_info(logger, "altHost: %s", value.get());
      }
      if (::strcmp("altPort", name) == 0){
        smppEntityInfo.altPort = atoi(value.get());
        smsc_log_info(logger, "altPort: %d", smppEntityInfo.altPort);
      }
    }

    smppEntityInfo.type = scag::transport::smpp::etSmsc;
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
