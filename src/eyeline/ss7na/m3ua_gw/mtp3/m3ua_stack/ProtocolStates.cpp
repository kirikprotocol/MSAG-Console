#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/sig/SctpEstablishInd.hpp"
#include "eyeline/ss7na/common/sig/SctpReleaseInd.hpp"

#include "messages/UPMessage.hpp"
#include "messages/UPAckMessage.hpp"
#include "messages/ActiveMessage.hpp"
#include "messages/ActiveAckMessage.hpp"
#include "messages/InactiveMessage.hpp"
#include "messages/InactiveAckMessage.hpp"
#include "messages/DownMessage.hpp"
#include "messages/DownAckMessage.hpp"
#include "messages/NotifyMessage.hpp"
#include "messages/ErrorMessage.hpp"

#include "ProtocolStates.hpp"

#include "logger/Logger.h"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

M3UA_State_NoConnection::M3UA_State_NoConnection()
  : _sctpConnectEstablishedInd_MessageCode(common::sig::SctpEstablishInd().getIndicationTypeValue()) {}

void
M3UA_State_NoConnection::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                   const common::Message& message)
{
  throw common::io_dispatcher::ProtocolException("M3UA_State_NoConnection::checkState::: transport connection wasn't established");
}

void
M3UA_State_NoConnection::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                   const common::io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_NoConnection::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectEstablishedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_NoConnection::checkState::: set new state M3UA_State_ASPDown");
    updateProtocolState(protocolController, M3UA_State_ASPDown::getInstance());
  }
}

M3UA_State_ASPDown::M3UA_State_ASPDown()
  : _upMessageCode (messages::UPMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()) {}

void
M3UA_State_ASPDown::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                              const common::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPDown::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _upMessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPDown::checkState::: set new state M3UA_State_ASPInactivePending");
    updateProtocolState(protocolController, M3UA_State_ASPInactivePending::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPDown::checkState::: protocol violation - unexpected message [=%s]", message.getMsgCodeTextDescription());
}

void
M3UA_State_ASPDown::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                              const common::io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPDown::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPDown::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPDown::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_Incoming_ManagmentMessages_Permitted::M3UA_Incoming_ManagmentMessages_Permitted()
  : _notifyMessageCode(messages::NotifyMessage().getMsgCode()),
    _errorMessageCode(messages::ErrorMessage().getMsgCode()) {}

void
M3UA_Incoming_ManagmentMessages_Permitted::checkIfGotManagementMessage(const common::Message& message, const std::string& where)
{
  uint32_t msgCode = message.getMsgCode();
  if ( msgCode != _notifyMessageCode &&
       msgCode != _errorMessageCode ) {
    std::string errMessage(where);
    errMessage += std::string("::: protocol violation - unexpected message [%s]");
    throw common::io_dispatcher::ProtocolException(errMessage.c_str(), message.getMsgCodeTextDescription());
  }
}

M3UA_State_ASPDownPending::M3UA_State_ASPDownPending()
  : _downAckMessageCode(messages::DownAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()) {}

void
M3UA_State_ASPDownPending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                     const common::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPDownPending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _downAckMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPDownPending::checkState::: set new state M3UA_State_ASPDown");
    updateProtocolState(protocolController, M3UA_State_ASPDown::getInstance());
  } else
    checkIfGotManagementMessage(message, "M3UA_State_ASPDownPending::checkState"); //can generate exception
}

void
M3UA_State_ASPDownPending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                     const common::io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPDownPending::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPDownPending::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPDownPending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_State_ASPInactivePending::M3UA_State_ASPInactivePending()
  : _upMessageAckCode(messages::UPAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()) {}

void
M3UA_State_ASPInactivePending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                         const common::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPInactivePending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _upMessageAckCode) {
    smsc_log_debug(logger, "M3UA_State_ASPInactivePending::checkState::: set new state M3UA_State_ASPInactive");
    updateProtocolState(protocolController, M3UA_State_ASPInactive::getInstance());
  } else
    checkIfGotManagementMessage(message, "M3UA_State_ASPInactivePending::checkState"); //can generate exception
}

void
M3UA_State_ASPInactivePending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                         const common::io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPInactivePending::checkState::: got indication=[%s]", indication.toString().c_str());

  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPInactivePending::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPInactivePending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_State_ASPInactive::M3UA_State_ASPInactive()
  : _activeMessageCode(messages::ActiveMessage().getMsgCode()),
    _downMessageCode(messages::DownMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue())
{}

void
M3UA_State_ASPInactive::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                  const common::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPInactive::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _activeMessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPInactive::checkState::: set new state M3UA_State_ASPActivePending");
    updateProtocolState(protocolController, M3UA_State_ASPActivePending::getInstance());
  } else if (message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPInactive::checkState::: set new state M3UA_State_ASPDownPending");
    updateProtocolState(protocolController, M3UA_State_ASPDownPending::getInstance());
  } else
    checkIfGotManagementMessage(message, "M3UA_State_ASPInactive::checkState"); //can generate exception
}

void
M3UA_State_ASPInactive::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                  const common::io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPInactive::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPInactive::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPInactive::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_State_ASPActivePending::M3UA_State_ASPActivePending()
  : _activeAckMessageCode(messages::ActiveAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()),
    _downMessageCode(messages::DownMessage().getMsgCode()) {}

void
M3UA_State_ASPActivePending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                       const common::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActivePending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _activeAckMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPActivePending::checkState::: set new state M3UA_State_ASPActive");
    updateProtocolState(protocolController, M3UA_State_ASPActive::getInstance());
  } else if ( message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPActivePending::checkState::: set new state M3UA_State_ASPDownPending");
    updateProtocolState(protocolController, M3UA_State_ASPDownPending::getInstance());
  } else
    checkIfGotManagementMessage(message, "M3UA_State_ASPActivePending::checkState"); //can generate exception
}

void
M3UA_State_ASPActivePending::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                       const common::io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActivePending::checkState::: got indication=[%s]", indication.toString().c_str());

  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPActivePending::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPActivePending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_State_ASPActive::M3UA_State_ASPActive()
  : _inactiveMessageCode(messages::InactiveMessage().getMsgCode()),
    _downMessageCode(messages::DownMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()) {}

void
M3UA_State_ASPActive::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                const common::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActive::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPActive::checkState::: set new state M3UA_State_ASPDownPending");
    updateProtocolState(protocolController, M3UA_State_ASPDownPending::getInstance());
  } else if ( message.getMsgCode() == _inactiveMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPActive::checkState::: set new state M3UA_State_ASPActiveShutdown");
    updateProtocolState(protocolController, M3UA_State_ASPActiveShutdown::getInstance());
  }
}

void
M3UA_State_ASPActive::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                const common::io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActive::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPActive::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPActive::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

M3UA_State_ASPActiveShutdown::M3UA_State_ASPActiveShutdown()
  : _inactiveAckMessageCode(messages::InactiveAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(common::sig::SctpReleaseInd().getIndicationTypeValue()) {}

void
M3UA_State_ASPActiveShutdown::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                        const common::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActiveShutdown::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _inactiveAckMessageCode ) {
    smsc_log_debug(logger, "M3UA_State_ASPActiveShutdown::checkState::: set new state M3UA_State_ASPInactive");
    updateProtocolState(protocolController, M3UA_State_ASPInactive::getInstance());
  } else
    checkIfGotManagementMessage(message, "M3UA_State_ASPActiveShutdown::checkState"); //can generate exception
}

void
M3UA_State_ASPActiveShutdown::checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                                        const common::io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("m3ua_stack");
  smsc_log_debug(logger, "M3UA_State_ASPActiveShutdown::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "M3UA_State_ASPActiveShutdown::checkState::: set new state M3UA_State_NoConnection");
    updateProtocolState(protocolController, M3UA_State_NoConnection::getInstance());
  } else
    throw common::io_dispatcher::ProtocolException("M3UA_State_ASPActiveShutdown::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

}}}}}
