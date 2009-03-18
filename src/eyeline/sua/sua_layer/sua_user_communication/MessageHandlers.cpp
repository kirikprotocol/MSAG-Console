#include <eyeline/sua/communication/LinkId.hpp>
#include <eyeline/sua/communication/libsua_messages/BindConfirmMessage.hpp>
#include <eyeline/sua/communication/libsua_messages/N_NOTICE_IND_Message.hpp>
#include <eyeline/sua/communication/libsua_messages/N_UNITDATA_IND_Message.hpp>
#include <eyeline/sua/communication/sua_messages/SCCPAddress.hpp>
#include <eyeline/sua/sua_layer/messages_router/GTTranslator.hpp>
#include <eyeline/sua/sua_layer/messages_router/Exception.hpp>

#include "LinkSetInfoRegistry.hpp"
#include "MessageHandlers.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

MessageHandlers::MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _linkSetInfoRegistry(LinkSetInfoRegistry::getInstance()) {}

void
MessageHandlers::handle(const communication::libsua_messages::BindMessage& message,
                        const communication::LinkId& linkId)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle BindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
    communication::LinkId linkSetId;
    if ( _linkSetInfoRegistry.getLinkSet(message.getAppId(), &linkSetId) ) {
      _cMgr.addLinkToLinkSet(linkSetId, linkId);
      _cMgr.send(linkId, communication::libsua_messages::BindConfirmMessage(communication::libsua_messages::BindConfirmMessage::BIND_OK));
    } else
      _cMgr.send(linkId, communication::libsua_messages::BindConfirmMessage(communication::libsua_messages::BindConfirmMessage::UNKNOWN_APP_ID_VALUE));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched unexpected exception [%s]", ex.what());
    _cMgr.send(linkId, communication::libsua_messages::BindConfirmMessage(communication::libsua_messages::BindConfirmMessage::SYSTEM_MALFUNCTION));
  }
}

void
MessageHandlers::handle(const communication::libsua_messages::UnbindMessage& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle UnbindMessage [%s] on link=[%s]", message.toString().c_str(), linkId.getValue().c_str());
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(linkId), linkId);
}

communication::sua_messages::CLDTMessage
MessageHandlers::createCLDTMessage(const communication::libsua_messages::N_UNITDATA_REQ_Message& message)
{
  communication::sua_messages::CLDTMessage messageForSending;

  uint32_t fakeIndexes[1] = {0};
  messageForSending.setRoutingContext(communication::sua_messages::TLV_RoutingContext(fakeIndexes, 1));

  const communication::libsua_messages::variable_data_t& calledAddr = message.getCalledAddress();
  communication::sua_messages::SCCPAddress sccpCalledAddr(calledAddr.data, calledAddr.dataLen);
  messageForSending.setDestinationAddress(sccpCalledAddr.make_TLV_Address<communication::sua_messages::TLV_DestinationAddress>());

  const communication::libsua_messages::variable_data_t& callingAddr = message.getCallingAddress();
  communication::sua_messages::SCCPAddress sccpCallingAddr(callingAddr.data, callingAddr.dataLen);
  messageForSending.setSourceAddress(sccpCallingAddr.make_TLV_Address<communication::sua_messages::TLV_SourceAddress>());

  communication::sua_messages::protocol_class_t protocolClassValue = communication::sua_messages::CLASS0_CONNECIONLESS;
  try {
    messageForSending.setSequenceControl(communication::sua_messages::TLV_SequenceControl(message.getSequenceControl()));
    protocolClassValue = communication::sua_messages::CLASS1_CONNECIONLESS;
  } catch (utilx::FieldNotSetException& ex) {
    messageForSending.setSequenceControl(communication::sua_messages::TLV_SequenceControl(0));
  }

  communication::sua_messages::ProtocolClass protocolClass(protocolClassValue, (message.getReturnOption() ? communication::sua_messages::RETURN_MSG_ON_ERROR : communication::sua_messages::NO_SPECIAL_OPTION));
  messageForSending.setProtocolClass(communication::sua_messages::TLV_ProtocolClass(protocolClass));

  try {
    messageForSending.setSS7HopCount(communication::sua_messages::TLV_SS7HopCount(message.getHopCounter()));
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    messageForSending.setImportance(communication::sua_messages::TLV_Importance(message.getImportance()));
  } catch (utilx::FieldNotSetException& ex) {}

  const communication::libsua_messages::variable_data_t& userData = message.getUserData();
  messageForSending.setData(communication::sua_messages::TLV_Data(userData.data, userData.dataLen));

  return messageForSending;
}

void
MessageHandlers::handle(const communication::libsua_messages::N_UNITDATA_REQ_Message& message,
                        const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle N_UNITDATA_REQ_Message [%s]", message.toString().c_str());

  try {
    const communication::sua_messages::CLDTMessage& cldtMmessage = createCLDTMessage(message);

    communication::LinkId outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldtMmessage.getDestinationAddress());

    std::string appId;
    if ( LinkSetInfoRegistry::getInstance().getAppId(outLinkSetId, &appId) ) {
      communication::libsua_messages::N_UNITDATA_IND_Message n_unitdata_ind_message(message);
      smsc_log_info(_logger, "MessageHandlers::handle::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]", n_unitdata_ind_message.toString().c_str(), outLinkSetId.getValue().c_str());

      _cMgr.send(outLinkSetId, n_unitdata_ind_message);
    } else {
      smsc_log_info(_logger, "MessageHandlers::handle::: send CLDT [%s] to link with linkid=[%s]", message.toString().c_str(), outLinkSetId.getValue().c_str());

      _cMgr.send(outLinkSetId, cldtMmessage);
    }
  } catch (messages_router::TranslationFailure& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(linkId, communication::libsua_messages::N_NOTICE_IND_Message(message, ex.getFailureCode()));
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: catched unexpected exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(linkId, communication::libsua_messages::N_NOTICE_IND_Message(message, communication::libsua_messages::N_NOTICE_IND_Message::UNQUALIFIED));
  }
}

}}}}
