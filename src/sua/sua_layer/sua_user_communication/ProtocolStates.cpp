#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>
#include <sua/communication/libsua_messages/BindMessage.hpp>
#include <sua/communication/libsua_messages/UnbindMessage.hpp>
#include <sua/communication/libsua_messages/MErrorMessage.hpp>

#include "ProtocolStates.hpp"
#include "TcpEstablishInd.hpp"
#include "TcpReleaseInd.hpp"

template<class T>
T* utilx::Singleton<T*>::_instance;

namespace sua_user_communication {

LibSua_State_NoConnection::LibSua_State_NoConnection()
  : _tcpConnectEstablishedInd_MessageCode(TcpEstablishInd().getIndicationTypeValue()) {}

void
LibSua_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message)
{
  throw io_dispatcher::ProtocolException("LibSua_State_NoConnection::checkState::: transport connection wasn't established");
}

void
LibSua_State_NoConnection::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication)
{
  if (indication.getIndicationTypeValue() == _tcpConnectEstablishedInd_MessageCode)
    updateProtocolState(protocolController, LibSua_State_Unbind::getInstance());
  else
    throw io_dispatcher::ProtocolException("LibSua_State_NoConnection::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

LibSua_State_Unbind::LibSua_State_Unbind()
  : _bindMessageCode (libsua_messages::BindMessage().getMsgCode()),
    _tcpConnectReleasedInd_MessageCode(TcpReleaseInd().getIndicationTypeValue()),
    _MErrorMessageCode(libsua_messages::MErrorMessage().getMsgCode()) {}

void
LibSua_State_Unbind::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message) {
  if (message.getMsgCode() == _bindMessageCode)
    updateProtocolState(protocolController, LibSua_State_Bind::getInstance());
  else if (message.getMsgCode() != _MErrorMessageCode)
    throw io_dispatcher::ProtocolException("LibSua_State_Unbind::checkState::: protocol violation - unexpected message  [=%s]", message.getMsgCodeTextDescription());
}

void
LibSua_State_Unbind::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication) {
  if (indication.getIndicationTypeValue() == _tcpConnectReleasedInd_MessageCode)
    updateProtocolState(protocolController, LibSua_State_NoConnection::getInstance());
  else
    throw io_dispatcher::ProtocolException("LibSua_State_Unbind::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

LibSua_State_Bind::LibSua_State_Bind()
  : _unbindMessageCode(libsua_messages::UnbindMessage().getMsgCode()),
    _tcpConnectReleasedInd_MessageCode(TcpReleaseInd().getIndicationTypeValue()) {}

void
LibSua_State_Bind::checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message) {
  if (message.getMsgCode() == _unbindMessageCode)
    updateProtocolState(protocolController, LibSua_State_Unbind::getInstance());
}

void
LibSua_State_Bind::checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication) {
  throw io_dispatcher::ProtocolException("LibSua_State_Bind::checkState::: protocol violation - unexpected indication [=%s]", indication.toString().c_str());
}

}
