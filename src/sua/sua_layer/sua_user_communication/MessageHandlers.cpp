#include <sua/communication/LinkId.hpp>
#include <sua/sua_layer/messages_router/MessagesRouter.hpp>
#include <sua/sua_layer/messages_router/RoutingKey.hpp>

#include "MessageHandlers.hpp"

namespace sua_user_communication {

MessageHandlers*
utilx::Singleton<MessageHandlers>::_instance;

MessageHandlers::MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _linkSetInfoRegistry(LinkSetInfoRegistry::getInstance()) {}

void
MessageHandlers::handle(const libsua_messages::BindMessage& message,
                        const communication::LinkId& linkId)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle BindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
    _cMgr.addLinkToLinkSet(_linkSetInfoRegistry.getLinkSet(message.getAppId()), linkId);

  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched unexpected exception [%s]", ex.what());
    _cMgr.send(linkId, libsua_messages::MErrorMessage(libsua_messages::MErrorMessage::NO_SESSION_ESTABLISHED, &message));
  }
}

void
MessageHandlers::handle(const libsua_messages::UnbindMessage& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle UnbindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(linkId), linkId);
}

void
MessageHandlers::handle(const libsua_messages::MErrorMessage& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle MErrorMessage [%s]", message.toString().c_str());
}

void
MessageHandlers::handle(const libsua_messages::EncapsulatedSuaMessage& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle EncapsulatedSuaMessage [%s]", message.toString().c_str());

  try {
    messages_router::RoutingKey routingKey;
    routingKey.setIncomingLinkId(linkId);

    const sua_messages::SUAMessage& messageForSending = *message.getContainedSuaMessage();
    const sua_messages::TLV_GlobalTitle& tlvGT = messageForSending.getDestinationAddress().getGlobalTitle();
    const sua_messages::GlobalTitle& gt = tlvGT.getGlobalTitle();
    const std::string& gtStringValue = gt.getGlobalTitleDigits();
    routingKey.setDestinationGT(gtStringValue);

    communication::LinkId outLinkId = messages_router::MessagesRouter::getInstance().getOutLink(routingKey);
    smsc_log_info(_logger, "MessageHandlers::handle::: send encapsulated SUAMessage [%s] to link with linkid=[%s]", message.toString().c_str(), outLinkId.getValue().c_str());

    _cMgr.send(outLinkId, messageForSending);
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched exception [%s], send MErrorMessage to messages's originator", ex.what());
    _cMgr.send(linkId, libsua_messages::MErrorMessage(libsua_messages::MErrorMessage::MESSAGE_FORWARDING_ERROR_TO_SGP, &message));
  }
}

}
