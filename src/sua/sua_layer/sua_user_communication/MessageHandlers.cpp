#include <sua/communication/LinkId.hpp>
#include <sua/communication/libsua_messages/BindConfirmMessage.hpp>
#include <sua/communication/libsua_messages/N_NOTICE_IND_Message.hpp>
#include <sua/communication/sua_messages/SCCPAddress.hpp>
#include <sua/sua_layer/messages_router/MessagesRouter.hpp>
#include <sua/sua_layer/messages_router/RoutingKey.hpp>

#include "MessageHandlers.hpp"

sua_user_communication::MessageHandlers*
utilx::Singleton<sua_user_communication::MessageHandlers>::_instance;

namespace sua_user_communication {

MessageHandlers::MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _linkSetInfoRegistry(LinkSetInfoRegistry::getInstance()) {}

void
MessageHandlers::handle(const libsua_messages::BindMessage& message,
                        const communication::LinkId& linkId)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle BindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
    communication::LinkId linkSetId;
    if ( _linkSetInfoRegistry.getLinkSet(message.getAppId(), &linkSetId) ) {
      _cMgr.addLinkToLinkSet(linkSetId, linkId);
      _cMgr.send(linkId, libsua_messages::BindConfirmMessage(libsua_messages::BindConfirmMessage::BIND_OK));
    } else
      _cMgr.send(linkId, libsua_messages::BindConfirmMessage(libsua_messages::BindConfirmMessage::UNKNOWN_APP_ID_VALUE));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched unexpected exception [%s]", ex.what());
    _cMgr.send(linkId, libsua_messages::BindConfirmMessage(libsua_messages::BindConfirmMessage::SYSTEM_MALFUNCTION));
  }
}

void
MessageHandlers::handle(const libsua_messages::UnbindMessage& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle UnbindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(linkId), linkId);
}

sua_messages::CLDTMessage
MessageHandlers::createCLDTMessage(const libsua_messages::N_UNITDATA_REQ_Message& message)
{
  sua_messages::CLDTMessage messageForSending;

  uint32_t fakeIndexes[1] = {0};
  messageForSending.setRoutingContext(sua_messages::TLV_RoutingContext(fakeIndexes, 1));

  const libsua_messages::variable_data_t& calledAddr = message.getCalledAddress();
  sua_messages::SCCPAddress sccpCalledAddr(calledAddr.data, calledAddr.dataLen);
  messageForSending.setDestinationAddress(sccpCalledAddr.make_TLV_Address<sua_messages::TLV_DestinationAddress>());

  const libsua_messages::variable_data_t& callingAddr = message.getCallingAddress();
  sua_messages::SCCPAddress sccpCallingAddr(callingAddr.data, callingAddr.dataLen);
  messageForSending.setSourceAddress(sccpCallingAddr.make_TLV_Address<sua_messages::TLV_SourceAddress>());

  sua_messages::protocol_class_t protocolClassValue = sua_messages::CLASS0_CONNECIONLESS;
  try {
    messageForSending.setSequenceControl(sua_messages::TLV_SequenceControl(message.getSequenceControl()));
    protocolClassValue = sua_messages::CLASS1_CONNECIONLESS;
  } catch (utilx::FieldNotSetException& ex) {
    messageForSending.setSequenceControl(sua_messages::TLV_SequenceControl(0));
  }

  sua_messages::ProtocolClass protocolClass(protocolClassValue, (message.getReturnOption() ? sua_messages::RETURN_MSG_ON_ERROR : sua_messages::NO_SPECIAL_OPTION));
  messageForSending.setProtocolClass(sua_messages::TLV_ProtocolClass(protocolClass));

  try {
    messageForSending.setSS7HopCount(sua_messages::TLV_SS7HopCount(message.getHopCounter()));
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    messageForSending.setImportance(sua_messages::TLV_Importance(message.getImportance()));
  } catch (utilx::FieldNotSetException& ex) {}

  const libsua_messages::variable_data_t& userData = message.getUserData();
  messageForSending.setData(sua_messages::TLV_Data(userData.data, userData.dataLen));

  return messageForSending;
}

void
MessageHandlers::handle(const libsua_messages::N_UNITDATA_REQ_Message& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle N_UNITDATA_REQ_Message [%s]", message.toString().c_str());

  try {
    const sua_messages::CLDTMessage& messageForSending = createCLDTMessage(message);

    const sua_messages::TLV_GlobalTitle& tlvGT = messageForSending.getDestinationAddress().getGlobalTitle();
    const sua_messages::GlobalTitle& gt = tlvGT.getGlobalTitle();
    const std::string& gtStringValue = gt.getGlobalTitleDigits();

    messages_router::RoutingKey routingKey;
    routingKey.setDestinationGT(gtStringValue);
    routingKey.setIncomingLinkId(linkId);

    communication::LinkId outLinkId = messages_router::MessagesRouter::getInstance().getOutLink(routingKey);
    smsc_log_info(_logger, "MessageHandlers::handle::: send CLDT [%s] to link with linkid=[%s]", message.toString().c_str(), outLinkId.getValue().c_str());

    communication::LinkId linkIdUsedForSending = _cMgr.send(outLinkId, messageForSending);
  } catch (utilx::NoRouteFound& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(linkId, libsua_messages::N_NOTICE_IND_Message(message, libsua_messages::N_NOTICE_IND_Message::NO_TRANSLATION_FOR_THIS_SPECIFIC_ADDRESS));
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched unexpected exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(linkId, libsua_messages::N_NOTICE_IND_Message(message, libsua_messages::N_NOTICE_IND_Message::UNQUALIFIED));
  }
}

}
