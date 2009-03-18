#include <util/Exception.hpp>

#include <eyeline/utilx/hexdmp.hpp>
#include <eyeline/utilx/PreallocatedMemoryManager.hpp>

#include <eyeline/sua/sua_layer/sua_user_communication/LibSuaConnect.hpp>
#include <eyeline/sua/sua_layer/io_dispatcher/Exceptions.hpp>
#include <eyeline/sua/communication/libsua_messages/BindMessage.hpp>
#include <eyeline/sua/communication/libsua_messages/UnbindMessage.hpp>

#include "ProtocolStates.hpp"
#include "TcpEstablishInd.hpp"
#include "TcpReleaseInd.hpp"

#include <logger/Logger.h>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

//Fake message - need just for holding msg code value
class CheckMessage : public communication::libsua_messages::LibsuaMessage {
public:
  CheckMessage(uint32_t msgCode) 
    : LibsuaMessage(msgCode) {
    snprintf(_msgcode_str, sizeof(_msgcode_str), "msgcode=[%d]", getMsgCode());
  }

  virtual const char* getMsgCodeTextDescription() const {
    return _msgcode_str; 
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const {}
protected:
  virtual uint32_t getLength() const { return 0; }

private:
  char _msgcode_str[32];
};

LibSuaConnect::LibSuaConnect(corex::io::network::TCPSocket* socket, const LibSuaConnectAcceptor* creator)
  : Link(LibSua_State_Unbind::getInstance()),
    _socket(socket), _isPassiveSocket(true), 
    _currentPacketSize(0), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _creator(creator)
{
  setLinkId(communication::LinkId(std::string("LINKID:") + socket->toString()));
}

LibSuaConnect::LibSuaConnect(const char* host, in_port_t port, const communication::LinkId& linkId)
  : Link(LibSua_State_NoConnection::getInstance()), _socket(new corex::io::network::TCPSocket(host, port)), _isPassiveSocket(false),
    _currentPacketSize(0), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _creator(NULL)
{
  setLinkId(linkId);
}

LibSuaConnect::~LibSuaConnect()
{
  delete _socket;
}

communication::TP*
LibSuaConnect::receive()
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_PACKET>();
  communication::TP* inTp = new (ptr_desc->allocated_memory) communication::TP();

  smsc_log_debug(_logger, "LibSuaConnect::receive::: _currentPacketSize=%d, _ringBuf.getSizeOfAvailData()=%d", _currentPacketSize, _ringBuf.getSizeOfAvailData());
  if ( _currentPacketSize > _ringBuf.getSizeOfAvailData() ) return NULL;

  inTp->packetLen = _currentPacketSize + sizeof(_currentPacketSize);
  memcpy(inTp->packetBody,  _header, sizeof(_header));
  _ringBuf.readArray(inTp->packetBody + sizeof(_header), _currentPacketSize);

  union {
    uint32_t msgCode;
    uint8_t msgCodeBuf[sizeof(uint32_t)];
  } bufForMsgCode;

  if ( _currentPacketSize < sizeof(bufForMsgCode.msgCode) )
    throw smsc::util::Exception("LibSuaConnect::receive::: can't extract msgcode value - buffer len  [=%d] is too small", _currentPacketSize);
  memcpy(bufForMsgCode.msgCodeBuf, inTp->packetBody + sizeof(_header), sizeof(bufForMsgCode.msgCode));
  bufForMsgCode.msgCode = ntohl(bufForMsgCode.msgCode);
  smsc_log_debug(_logger, "LibSuaConnect::receive::: _ringBuf.readArray() returned buf=[%s]", utilx::hexdmp(inTp->packetBody,inTp->packetLen).c_str());

  changeProtocolState(CheckMessage(bufForMsgCode.msgCode));
  // TODO: FIX IT
//   try {
//     changeProtocolState(CheckMessage(bufForMsgCode.msgCode));
//   } catch (const io_dispatcher::ProtocolException& ex) {
//     send(libsua_messages::N_NOTICE_Message(libsua_messages::MErrorMessage::NO_SESSION_ESTABLISHED, inTp));
//     throw;
//   }
  inTp->packetType = communication::libsua_messages::LibsuaMessage::getMessageIndex(bufForMsgCode.msgCode);

  smsc_log_debug(_logger, "LibSuaConnect::receive::: inTp->packetType=%d", inTp->packetType);
  if ( _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
    _ringBuf.readArray(_header, sizeof(_header));
    memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header, sizeof(_currentPacketSize));
    _currentPacketSize = ntohl(_currentPacketSize);
    if ( _currentPacketSize + sizeof(_header) > communication::TP::MAX_PACKET_SIZE )
      throw smsc::util::Exception("LibSuaConnect::receive::: length [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize + sizeof(_header), communication::TP::MAX_PACKET_SIZE);
  } else
    _currentPacketSize = 0;

  return inTp;
}

bool
LibSuaConnect::hasReadyTransportPacket()
{
  return _currentPacketSize > 0 && _currentPacketSize <= _ringBuf.getSizeOfAvailData();
}

void
LibSuaConnect::bufferInputTransportPackets()
{
  if ( _currentPacketSize == 0  ||
       _currentPacketSize > _ringBuf.getSizeOfAvailData() ) {
    _ringBuf.load(_socket->getInputStream()); // read as many bytes as possible into buffer from input stream
    if ( _currentPacketSize == 0 && _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
      _ringBuf.readArray(_header, sizeof(_header));
      memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header, sizeof(_currentPacketSize));
      _currentPacketSize = ntohl(_currentPacketSize);
      if ( _currentPacketSize + sizeof(_header) > communication::TP::MAX_PACKET_SIZE )
        throw smsc::util::Exception("LibSuaConnect::bufferInputTransportPackets::: length [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize + sizeof(_header), communication::TP::MAX_PACKET_SIZE);
    }
  }
}

void
LibSuaConnect::bind(const char* appId)
{
  if ( _isPassiveSocket )
    throw smsc::util::Exception("LibSuaConnect::bind::: can't bind incoming session");

  communication::libsua_messages::BindMessage bindMessage;
  bindMessage.setAppId(appId);

  send(bindMessage);
}

void
LibSuaConnect::unbind()
{
  if ( _isPassiveSocket )
    throw smsc::util::Exception("LibSuaConnect::unbind::: can't unbind incoming session");

  communication::libsua_messages::UnbindMessage unbindMessage;

  send(unbindMessage);
}

void
LibSuaConnect::tcpEstablish()
{
  if ( !_isPassiveSocket ) {
    changeProtocolState(TcpEstablishInd());
    _socket->connect();
  }
}

void
LibSuaConnect::tcpRelease()
{
  if ( !_isPassiveSocket ) {
    changeProtocolState(TcpReleaseInd());
    _socket->close();
  } else
    throw smsc::util::Exception("LibSuaConnect::tcpRelease::: can't release incoming session");
}

corex::io::network::Socket*
LibSuaConnect::_getSocket() const { return _socket; }

corex::io::OutputStream*
LibSuaConnect::getOutputStream() const
{
  return _socket->getOutputStream();
}

const io_dispatcher::ConnectAcceptor*
LibSuaConnect::getCreator() const
{
  return _creator;
}

}}}}
