#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>

#include <sua/communication/sua_messages/UPMessage.hpp>
#include <sua/communication/sua_messages/UPAckMessage.hpp>
#include <sua/communication/sua_messages/ActiveMessage.hpp>
#include <sua/communication/sua_messages/ActiveAckMessage.hpp>
#include <sua/communication/sua_messages/InactiveMessage.hpp>
#include <sua/communication/sua_messages/InactiveAckMessage.hpp>
#include <sua/communication/sua_messages/DownMessage.hpp>
#include <sua/communication/sua_messages/DownAckMessage.hpp>
#include <sua/communication/sua_messages/NotifyMessage.hpp>
#include <sua/communication/sua_messages/ErrorMessage.hpp>

#include "ProtocolStates.hpp"
#include "SctpEstablishInd.hpp"
#include "SctpReleaseInd.hpp"

#include <logger/Logger.h>

namespace sua_stack {

SUA_State_NoConnection::SUA_State_NoConnection()
  : _sctpConnectEstablishedInd_MessageCode(SctpEstablishInd().getIndicationTypeValue()) {}

void
SUA_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  throw io_dispatcher::ProtocolException("SUA_State_NoConnection::checkState::: transport connection wasn't established");
}

void
SUA_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_NoConnection::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectEstablishedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_NoConnection::checkState::: set new state SUA_State_ASPDown");
    updateProtocolState(protocolController, SUA_State_ASPDown::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_NoConnection::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPDown::SUA_State_ASPDown()
  : _upMessageCode (sua_messages::UPMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()) {}

void
SUA_State_ASPDown::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPDown::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _upMessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPDown::checkState::: set new state SUA_State_ASPInactivePending");
    updateProtocolState(protocolController, SUA_State_ASPInactivePending::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPDown::checkState::: protocol violation - unexpected message [=%s]", message.getMsgCodeTextDescription());
}

void
SUA_State_ASPDown::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPDown::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPDown::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPDown::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_Incoming_ManagmentMessages_Permitted::SUA_Incoming_ManagmentMessages_Permitted()
  : _notifyMessageCode(sua_messages::NotifyMessage().getMsgCode()),
    _errorMessageCode(sua_messages::ErrorMessage().getMsgCode()) {}

void
SUA_Incoming_ManagmentMessages_Permitted::checkIfGotManagementMessage(const communication::Message& message, const std::string& where)
{
  uint32_t msgCode = message.getMsgCode();
  if ( msgCode != _notifyMessageCode &&
       msgCode != _errorMessageCode ) {
    std::string errMessage(where);
    errMessage += std::string("::: protocol violation - unexpected message [%s]");
    throw io_dispatcher::ProtocolException(errMessage.c_str(), message.getMsgCodeTextDescription());
  }
}

SUA_State_ASPDownPending::SUA_State_ASPDownPending()
  : _downAckMessageCode(sua_messages::DownAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()) {}

void
SUA_State_ASPDownPending::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPDownPending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _downAckMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPDownPending::checkState::: set new state SUA_State_ASPDown");
    updateProtocolState(protocolController, SUA_State_ASPDown::getInstance());
  } else
    checkIfGotManagementMessage(message, "SUA_State_ASPDownPending::checkState"); //can generate exception
}

void
SUA_State_ASPDownPending::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPDownPending::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPDownPending::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPDownPending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPInactivePending::SUA_State_ASPInactivePending()
  : _upMessageAckCode(sua_messages::UPAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()) {}

void
SUA_State_ASPInactivePending::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPInactivePending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _upMessageAckCode) {
    smsc_log_debug(logger, "SUA_State_ASPInactivePending::checkState::: set new state SUA_State_ASPInactive");
    updateProtocolState(protocolController, SUA_State_ASPInactive::getInstance());
  } else
    checkIfGotManagementMessage(message, "SUA_State_ASPInactivePending::checkState"); //can generate exception
}

void
SUA_State_ASPInactivePending::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPInactivePending::checkState::: got indication=[%s]", indication.toString().c_str());

  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPInactivePending::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPInactivePending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPInactive::SUA_State_ASPInactive()
  : _activeMessageCode(sua_messages::ActiveMessage().getMsgCode()),
    _downMessageCode(sua_messages::DownMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue())
{}

void
SUA_State_ASPInactive::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPInactive::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if (message.getMsgCode() == _activeMessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPInactive::checkState::: set new state SUA_State_ASPActivePending");
    updateProtocolState(protocolController, SUA_State_ASPActivePending::getInstance());
  } else if (message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPInactive::checkState::: set new state SUA_State_ASPDownPending");
    updateProtocolState(protocolController, SUA_State_ASPDownPending::getInstance());
  } else
    checkIfGotManagementMessage(message, "SUA_State_ASPInactive::checkState"); //can generate exception
}

void
SUA_State_ASPInactive::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication) {
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPInactive::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPInactive::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPInactive::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPActivePending::SUA_State_ASPActivePending()
  : _activeAckMessageCode(sua_messages::ActiveAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()),
    _downMessageCode(sua_messages::DownMessage().getMsgCode()) {}

void
SUA_State_ASPActivePending::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActivePending::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _activeAckMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPActivePending::checkState::: set new state SUA_State_ASPActive");
    updateProtocolState(protocolController, SUA_State_ASPActive::getInstance());
  } else if ( message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPActivePending::checkState::: set new state SUA_State_ASPDownPending");
    updateProtocolState(protocolController, SUA_State_ASPDownPending::getInstance());
  } else
    checkIfGotManagementMessage(message, "SUA_State_ASPActivePending::checkState"); //can generate exception
}

void
SUA_State_ASPActivePending::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActivePending::checkState::: got indication=[%s]", indication.toString().c_str());

  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPActivePending::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPActivePending::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPActive::SUA_State_ASPActive()
  : _inactiveMessageCode(sua_messages::InactiveMessage().getMsgCode()),
    _downMessageCode(sua_messages::DownMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()) {}

void
SUA_State_ASPActive::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActive::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _downMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPActive::checkState::: set new state SUA_State_ASPDownPending");
    updateProtocolState(protocolController, SUA_State_ASPDownPending::getInstance());
  } else if ( message.getMsgCode() == _inactiveMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPActive::checkState::: set new state SUA_State_ASPActiveShutdown");
    updateProtocolState(protocolController, SUA_State_ASPActiveShutdown::getInstance());
  }
}

void
SUA_State_ASPActive::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActive::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPActive::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPActive::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

SUA_State_ASPActiveShutdown::SUA_State_ASPActiveShutdown()
  : _inactiveAckMessageCode(sua_messages::InactiveAckMessage().getMsgCode()),
    _sctpConnectReleasedInd_MessageCode(SctpReleaseInd().getIndicationTypeValue()) {}

void
SUA_State_ASPActiveShutdown::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActiveShutdown::checkState::: got message=[%s]", message.getMsgCodeTextDescription());
  if ( message.getMsgCode() == _inactiveAckMessageCode ) {
    smsc_log_debug(logger, "SUA_State_ASPActiveShutdown::checkState::: set new state SUA_State_ASPInactive");
    updateProtocolState(protocolController, SUA_State_ASPInactive::getInstance());
  } else
    checkIfGotManagementMessage(message, "SUA_State_ASPActiveShutdown::checkState"); //can generate exception
}

void
SUA_State_ASPActiveShutdown::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sua_stack");
  smsc_log_debug(logger, "SUA_State_ASPActiveShutdown::checkState::: got indication=[%s]", indication.toString().c_str());
  if (indication.getIndicationTypeValue() == _sctpConnectReleasedInd_MessageCode) {
    smsc_log_debug(logger, "SUA_State_ASPActiveShutdown::checkState::: set new state SUA_State_NoConnection");
    updateProtocolState(protocolController, SUA_State_NoConnection::getInstance());
  } else
    throw io_dispatcher::ProtocolException("SUA_State_ASPActiveShutdown::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

}
