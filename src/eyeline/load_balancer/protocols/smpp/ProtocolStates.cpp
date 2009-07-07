#include "ProtocolStates.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "AlertNotification.hpp"
#include "BindReceiver.hpp"
#include "BindReceiverResp.hpp"
#include "BindTransceiver.hpp"
#include "BindTransceiverResp.hpp"
#include "BindTransmitter.hpp"
#include "BindTransmitterResp.hpp"
#include "CancelSm.hpp"
#include "CancelSmResp.hpp"
#include "DataSm.hpp"
#include "DataSmResp.hpp"
#include "DeliverSm.hpp"
#include "DeliverSmResp.hpp"
#include "EnquireLink.hpp"
#include "EnquireLinkResp.hpp"
#include "GenericNack.hpp"
#include "QuerySm.hpp"
#include "QuerySmResp.hpp"
#include "ReplaceSm.hpp"
#include "ReplaceSmResp.hpp"
#include "SubmitMulti.hpp"
#include "SubmitMultiResp.hpp"
#include "SubmitSm.hpp"
#include "SubmitSmResp.hpp"
#include "Unbind.hpp"
#include "UnbindResp.hpp"
#include "eyeline/utilx/prot_fsm/TcpEstablishInd.hpp"
#include "eyeline/utilx/prot_fsm/TcpReleaseInd.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SMPP_NotConnected::checkState(SMPPProtocolStateController* protocol_state_controller,
                              const SMPPMessage& message)
{
  throw utilx::ProtocolException("SMPP_NotConnected::checkState::: transport connection isn't established");
}

void
SMPP_NotConnected::checkState(SMPPProtocolStateController* protocol_state_controller,
                              const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpEstablishInd::TCP_ESTABLISH_IND)
    updateProtocolState(protocol_state_controller, SMPP_Connected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_NotConnected::checkState::: protocol violation - unexpected indication=[%s]", indication.toString().c_str());
}

void
SMPP_Connected::checkState(SMPPProtocolStateController* protocol_state_controller,
                           const SMPPMessage& message)
{
  if (message.getMsgCode() == BindReceiver::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Binding_RX::getInstance());
  else if (message.getMsgCode() == BindTransceiver::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Binding_TRX::getInstance());
  else if (message.getMsgCode() == BindTransmitter::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Binding_TX::getInstance());
  else if (message.getMsgCode() != EnquireLink::_MSGCODE && message.getMsgCode() != EnquireLinkResp::_MSGCODE)
    throw utilx::ProtocolException("SMPP_Connected::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
}

void
SMPP_Connected::checkState(SMPPProtocolStateController* protocol_state_controller,
                           const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Connected::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Binding_RX::checkState(SMPPProtocolStateController* protocol_state_controller,
                            const SMPPMessage& message)
{
  if (message.getMsgCode() == BindReceiverResp::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Bound_RX::getInstance());
  else if (message.getMsgCode() != EnquireLink::_MSGCODE && message.getMsgCode() != EnquireLinkResp::_MSGCODE)
    throw utilx::ProtocolException("SMPP_Binding_RX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
}

void
SMPP_Binding_RX::checkState(SMPPProtocolStateController* protocol_state_controller,
                            const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Binding_RX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Binding_TRX::checkState(SMPPProtocolStateController* protocol_state_controller,
                             const SMPPMessage& message)
{
  if (message.getMsgCode() == BindTransceiverResp::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Bound_TRX::getInstance());
  else if (message.getMsgCode() != EnquireLink::_MSGCODE && message.getMsgCode() != EnquireLinkResp::_MSGCODE)
    throw utilx::ProtocolException("SMPP_Binding_TRX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
}

void
SMPP_Binding_TRX::checkState(SMPPProtocolStateController* protocol_state_controller,
                            const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Binding_TRX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Binding_TX::checkState(SMPPProtocolStateController* protocol_state_controller,
                            const SMPPMessage& message)
{
  if (message.getMsgCode() == BindTransmitterResp::_MSGCODE)
    updateProtocolState(protocol_state_controller, SMPP_Bound_TX::getInstance());
  else if (message.getMsgCode() != EnquireLink::_MSGCODE && message.getMsgCode() != EnquireLinkResp::_MSGCODE)
    throw utilx::ProtocolException("SMPP_Binding_TX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
}

void
SMPP_Binding_TX::checkState(SMPPProtocolStateController* protocol_state_controller,
                            const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Binding_TX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Bound_RX::checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message)
{
  io_subsystem::Message::message_code_t msgCode = message.getMsgCode();
  switch ( msgCode ) {
  case DataSm::_MSGCODE:
  case DataSmResp::_MSGCODE:
  case DeliverSm::_MSGCODE:
  case DeliverSmResp::_MSGCODE:
  case EnquireLink::_MSGCODE:
  case EnquireLinkResp::_MSGCODE:
  case AlertNotification::_MSGCODE:
  case GenericNack::_MSGCODE:
    break;
  case Unbind::_MSGCODE:
    updateProtocolState(protocol_state_controller, SMPP_Unbound::getInstance());
    break;
  default:
    throw utilx::ProtocolException("SMPP_Bound_RX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
  }
}

void
SMPP_Bound_RX::checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Bound_RX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Bound_TRX::checkState(SMPPProtocolStateController* protocol_state_controller,
                           const SMPPMessage& message)
{
  io_subsystem::Message::message_code_t msgCode = message.getMsgCode();
  switch ( msgCode ) {
  case DataSm::_MSGCODE:
  case DataSmResp::_MSGCODE:
  case DeliverSm::_MSGCODE:
  case DeliverSmResp::_MSGCODE:
  case SubmitSm::_MSGCODE:
  case SubmitSmResp::_MSGCODE:
  case SubmitMulti::_MSGCODE:
  case SubmitMultiResp::_MSGCODE:
  case CancelSm::_MSGCODE:
  case CancelSmResp::_MSGCODE:
  case QuerySm::_MSGCODE:
  case QuerySmResp::_MSGCODE:
  case EnquireLink::_MSGCODE:
  case EnquireLinkResp::_MSGCODE:
  case AlertNotification::_MSGCODE:
  case GenericNack::_MSGCODE:
    break;
  case Unbind::_MSGCODE:
    updateProtocolState(protocol_state_controller, SMPP_Unbound::getInstance());
    break;
  default:
    throw utilx::ProtocolException("SMPP_Bound_TRX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
  }
}

void
SMPP_Bound_TRX::checkState(SMPPProtocolStateController* protocol_state_controller,
                           const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Bound_TRX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Bound_TX::checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message)
{
  io_subsystem::Message::message_code_t msgCode = message.getMsgCode();
  switch ( msgCode ) {
  case DataSm::_MSGCODE:
  case DataSmResp::_MSGCODE:
  case SubmitSm::_MSGCODE:
  case SubmitSmResp::_MSGCODE:
  case SubmitMulti::_MSGCODE:
  case SubmitMultiResp::_MSGCODE:
  case CancelSm::_MSGCODE:
  case CancelSmResp::_MSGCODE:
  case QuerySm::_MSGCODE:
  case QuerySmResp::_MSGCODE:
  case ReplaceSm::_MSGCODE:
  case ReplaceSmResp::_MSGCODE:
  case EnquireLink::_MSGCODE:
  case EnquireLinkResp::_MSGCODE:
  case GenericNack::_MSGCODE:
    break;
  case Unbind::_MSGCODE:
    updateProtocolState(protocol_state_controller, SMPP_Unbound::getInstance());
    break;
  default:
    throw utilx::ProtocolException("SMPP_Bound_TX::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
  }
}

void
SMPP_Bound_TX::checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Bound_TX::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

void
SMPP_Unbound::checkState(SMPPProtocolStateController* protocol_state_controller,
                         const SMPPMessage& message)
{
  if (message.getMsgCode() != UnbindResp::_MSGCODE &&
      message.getMsgCode() != EnquireLink::_MSGCODE &&
      message.getMsgCode() != EnquireLinkResp::_MSGCODE)
    throw utilx::ProtocolException("SMPP_Unbound::checkState::: protocol violation - unexpected message=[%s]", message.toString().c_str());
}

void
SMPP_Unbound::checkState(SMPPProtocolStateController* protocol_state_controller,
                         const utilx::prot_fsm::TCPIndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == utilx::prot_fsm::TcpReleaseInd::TCP_RELEASE_IND)
    updateProtocolState(protocol_state_controller, SMPP_NotConnected::getInstance());
  else
    throw utilx::ProtocolException("SMPP_Unbound::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

}}}}
