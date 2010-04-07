#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"

#include "ProtocolStates.hpp"
#include "TcpEstablishInd.hpp"
#include "TcpReleaseInd.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

LibSccp_State_NoConnection::LibSccp_State_NoConnection()
  : _tcpConnectEstablishedInd_MessageCode(TcpEstablishInd().getIndicationTypeValue()) {}

void
LibSccp_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                                      const Message& message)
{
  throw common::io_dispatcher::ProtocolException("LibSccp_State_NoConnection::checkState::: transport connection wasn't established");
}

void
LibSccp_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                                      const io_dispatcher::IndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == _tcpConnectEstablishedInd_MessageCode)
    updateProtocolState(protocol_controller, LibSccp_State_Unbind::getInstance());
  else
    throw io_dispatcher::ProtocolException("LibSccp_State_NoConnection::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

LibSccp_State_Unbind::LibSccp_State_Unbind()
  : _bindMessageCode (libsccp::BindMessage().getMsgCode()),
    _tcpConnectReleasedInd_MessageCode(TcpReleaseInd().getIndicationTypeValue())
{}

void
LibSccp_State_Unbind::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                                const Message& message) {
  if (message.getMsgCode() == _bindMessageCode)
    updateProtocolState(protocol_controller, LibSccp_State_Bind::getInstance());
}

void
LibSccp_State_Unbind::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                                const io_dispatcher::IndicationPrimitive& indication) {
  if (indication.getIndicationTypeValue() == _tcpConnectReleasedInd_MessageCode)
    updateProtocolState(protocol_controller, LibSccp_State_NoConnection::getInstance());
  else
    throw io_dispatcher::ProtocolException("LibSccp_State_Unbind::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

LibSccp_State_Bind::LibSccp_State_Bind()
  : _unbindMessageCode(libsccp::UnbindMessage().getMsgCode()),
    _tcpConnectReleasedInd_MessageCode(TcpReleaseInd().getIndicationTypeValue()) {}

void
LibSccp_State_Bind::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                              const Message& message) {
  if (message.getMsgCode() == _unbindMessageCode)
    updateProtocolState(protocol_controller, LibSccp_State_Unbind::getInstance());
}

void
LibSccp_State_Bind::checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                              const io_dispatcher::IndicationPrimitive& indication) {
  throw io_dispatcher::ProtocolException("LibSccp_State_Bind::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

}}}}
