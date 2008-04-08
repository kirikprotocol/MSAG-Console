#include <sua/communication/LinkId.hpp>
#include <sua/sua_layer/messages_router/MessagesRouter.hpp>
#include <sua/sua_layer/messages_router/RoutingKey.hpp>
#include <sua/communication/libsua_messages/EncapsulatedSuaMessage.hpp>

#include "CLCOMessageHandlers.hpp"

namespace sua_stack {

CLCOMessageHandlers*
utilx::Singleton<CLCOMessageHandlers>::_instance;

CLCOMessageHandlers::CLCOMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance())
{}

void
CLCOMessageHandlers::handle(const sua_messages::CLDTMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "CLCOMessageHandlers::handle::: handle CLDTMessage=[%s]", message.toString().c_str());
  messages_router::RoutingKey routingKey;
  routingKey.setIncomingLinkId(linkId);

  const sua_messages::TLV_GlobalTitle& tlvGT = message.getDestinationAddress().getGlobalTitle();
  const sua_messages::GlobalTitle& gt = tlvGT.getGlobalTitle();
  const std::string& gtStringValue = gt.getGlobalTitleDigits();
  routingKey.setDestinationGT(gtStringValue);

  bool callTerminationIndicator;
  communication::LinkId outLinkId = messages_router::MessagesRouter::getInstance().getOutLink(routingKey, &callTerminationIndicator);
  if ( callTerminationIndicator ) {
    libsua_messages::EncapsulatedSuaMessage messageForSending(&message);
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: send encapsulated CLDTMessage [%s] to link with linkid=[%s]", messageForSending.toString().c_str(), outLinkId.getValue().c_str());
    _cMgr.send(outLinkId, messageForSending);
  } else {
    smsc_log_info(_logger, "CLCOMessageHandlers::handle::: forward original CLDT message=[%s] to next SGP over link with linkid=[%s]", message.toString().c_str(), outLinkId.getValue().c_str());
    _cMgr.send(outLinkId, message);
  }
}

}
