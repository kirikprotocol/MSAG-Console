#include "MessageHandlers.hpp"

#include "eyeline/utilx/types.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"
#include "eyeline/ss7na/sua_gw/sua_stack/messages/SCCPAddress.hpp"
#include "eyeline/ss7na/sua_gw/messages_router/GTTranslator.hpp"

#include "messages/N_UNITDATA_IND_Message.hpp"
#include "messages/N_NOTICE_IND_Message.hpp"
#include "eyeline/ss7na/libsccp/messages/BindConfirmMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

void
MessageHandlers::handle(const libsccp::BindMessage& message,
                        const common::LinkId& link_id)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle BindMessage [%s] on link=[%s]", message.toString().c_str(), link_id.getValue().c_str());
    common::LinkId linkSetId;
    common::sccp_sap::ActiveAppsRegistry::getInstance().insert(message.getSSNList(),
                                                               message.getSSNList() + message.getNumberOfSSN(),
                                                               link_id, message.getAppId());

    if ( _linkSetInfoRegistry.getLinkSet(message.getAppId(), &linkSetId) ) {
      _cMgr.addLinkToLinkSet(linkSetId, link_id);
      _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::BIND_OK));
    } else
      _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::UNKNOWN_APP_ID_VALUE));
  } catch (utilx::DuplicatedRegistryKeyException& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught DuplicatedRegistryKeyException [%s]", ex.what());
    _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::APP_ALREADY_ACTIVE));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception [%s]", ex.what());
    _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::SYSTEM_MALFUNCTION));
  }
}

void
MessageHandlers::handle(const libsccp::N_UNITDATA_REQ_Message& message,
                        const common::LinkId& link_id)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle N_UNITDATA_REQ_Message [%s]", message.toString().c_str());

  try {
    const sua_stack::messages::CLDTMessage& cldtMessage = createCLDTMessage(message);

    common::LinkId outLinkSetId = messages_router::GTTranslator::getInstance().translate(cldtMessage.getDestinationAddress());

    std::string appId;
    if ( _linkSetInfoRegistry.getAppId(outLinkSetId, &appId) ) {
      messages::N_UNITDATA_IND_Message nUnitdataIndMessage(message);
      smsc_log_info(_logger, "MessageHandlers::handle::: send N_UNITDATA_IND_Message [%s] to link with linkid=[%s]",
                    nUnitdataIndMessage.toString().c_str(), outLinkSetId.getValue().c_str());

      _cMgr.send(outLinkSetId, nUnitdataIndMessage);
    } else {
      smsc_log_info(_logger, "MessageHandlers::handle::: send CLDT [%s] to link with linkid=[%s]", message.toString().c_str(), outLinkSetId.getValue().c_str());

      _cMgr.send(outLinkSetId, cldtMessage);
    }
  } catch (common::TranslationFailure& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(link_id, messages::N_NOTICE_IND_Message(message, ex.getFailureCode()));
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception [%s], send N_NOTICE_IND_Message to messages's originator", ex.what());
    _cMgr.send(link_id, messages::N_NOTICE_IND_Message(message, libsccp::N_NOTICE_IND_Message::UNQUALIFIED));
  }
}

void
MessageHandlers::handle(const libsccp::UnbindMessage& message,
                        const common::LinkId& link_id)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle UnbindMessage [%s] on link=[%s]", message.toString().c_str(), link_id.getValue().c_str());
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(link_id), link_id);
  common::sccp_sap::ActiveAppsRegistry::getInstance().remove(link_id);
}

sua_stack::messages::CLDTMessage
MessageHandlers::createCLDTMessage(const libsccp::N_UNITDATA_REQ_Message& message)
{
  sua_stack::messages::CLDTMessage messageForSending;

  uint32_t fakeIndexes[1] = {0};
  messageForSending.setRoutingContext(common::TLV_RoutingContext(fakeIndexes, 1));

  const utilx::variable_data_t& calledAddr = message.getCalledAddress();
  sua_stack::messages::SCCPAddress sccpCalledAddr(calledAddr.data, calledAddr.dataLen);
  messageForSending.setDestinationAddress(sccpCalledAddr.make_TLV_Address<sua_stack::messages::TLV_DestinationAddress>());

  const utilx::variable_data_t& callingAddr = message.getCallingAddress();
  sua_stack::messages::SCCPAddress sccpCallingAddr(callingAddr.data, callingAddr.dataLen);
  messageForSending.setSourceAddress(sccpCallingAddr.make_TLV_Address<sua_stack::messages::TLV_SourceAddress>());

  sua_stack::messages::protocol_class_t protocolClassValue = sua_stack::messages::CLASS0_CONNECIONLESS;
  try {
    messageForSending.setSequenceControl(sua_stack::messages::TLV_SequenceControl(message.getSequenceControl()));
    protocolClassValue = sua_stack::messages::CLASS1_CONNECIONLESS;
  } catch (utilx::FieldNotSetException& ex) {
    messageForSending.setSequenceControl(sua_stack::messages::TLV_SequenceControl(0));
  }

  sua_stack::messages::ProtocolClass protocolClass(protocolClassValue, (message.getReturnOption() ? sua_stack::messages::RETURN_MSG_ON_ERROR : sua_stack::messages::NO_SPECIAL_OPTION));
  messageForSending.setProtocolClass(sua_stack::messages::TLV_ProtocolClass(protocolClass));

  try {
    messageForSending.setSS7HopCount(sua_stack::messages::TLV_SS7HopCount(message.getHopCounter()));
  } catch (utilx::FieldNotSetException& ex) {}

  try {
    messageForSending.setImportance(sua_stack::messages::TLV_Importance(message.getImportance()));
  } catch (utilx::FieldNotSetException& ex) {}

  const utilx::variable_data_t& userData = message.getUserData();
  messageForSending.setData(sua_stack::messages::TLV_Data(userData.data, userData.dataLen));

  return messageForSending;
}

}}}}
