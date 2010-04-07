#include "util/Exception.hpp"

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/sccp_sap/LibSccpConnect.hpp"
#include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"

#include "ProtocolStates.hpp"
#include "TcpEstablishInd.hpp"
#include "TcpReleaseInd.hpp"

#include "logger/Logger.h"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

//Fake message - need just for holding msg code value
class CheckMessage : public libsccp::LibsccpMessage {
public:
  CheckMessage(uint32_t msg_code)
    : LibsccpMessage(msg_code) {
    snprintf(_msgcode_str, sizeof(_msgcode_str), "msgcode=[%d]", getMsgCode());
  }

  virtual const char* getMsgCodeTextDescription() const {
    return _msgcode_str; 
  }

  virtual void dispatch_handle(const LinkId& link_id) const {}
protected:
  virtual uint32_t getLength() const { return 0; }

private:
  char _msgcode_str[32];
};

LibSccpConnect::LibSccpConnect(corex::io::network::TCPSocket* socket,
                             const LibSccpConnectAcceptor* creator)
  : Link(LibSccp_State_Unbind::getInstance()),
    _socket(socket), _isPassiveSocket(true), 
    _currentPacketSize(0), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _creator(creator)
{
  setLinkId(LinkId(std::string("LINKID:") + socket->toString()));
}

LibSccpConnect::LibSccpConnect(const char* host,
                             in_port_t port,
                             const LinkId& link_id)
  : Link(LibSccp_State_NoConnection::getInstance()), _socket(new corex::io::network::TCPSocket(host, port)), _isPassiveSocket(false),
    _currentPacketSize(0), _logger(smsc::logger::Logger::getInstance("sua_usr_cm")), _creator(NULL)
{
  setLinkId(link_id);
}

LibSccpConnect::~LibSccpConnect()
{
  delete _socket;
}

TP*
LibSccpConnect::receive()
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_PACKET>();
  TP* inTp = new (ptr_desc->allocated_memory) TP();

  smsc_log_debug(_logger, "LibSccpConnect::receive::: _currentPacketSize=%d, _ringBuf.getSizeOfAvailData()=%d", _currentPacketSize, _ringBuf.getSizeOfAvailData());
  if ( _currentPacketSize > _ringBuf.getSizeOfAvailData() ) return NULL;

  inTp->packetLen = _currentPacketSize + sizeof(_currentPacketSize);
  memcpy(inTp->packetBody,  _header, sizeof(_header));
  _ringBuf.readArray(inTp->packetBody + sizeof(_header), _currentPacketSize);

  union {
    uint32_t msgCode;
    uint8_t msgCodeBuf[sizeof(uint32_t)];
  } bufForMsgCode;

  if ( _currentPacketSize < sizeof(bufForMsgCode.msgCode) )
    throw smsc::util::Exception("LibSccpConnect::receive::: can't extract msgcode value - buffer len  [=%d] is too small", _currentPacketSize);
  memcpy(bufForMsgCode.msgCodeBuf, inTp->packetBody + sizeof(_header), sizeof(bufForMsgCode.msgCode));
  bufForMsgCode.msgCode = ntohl(bufForMsgCode.msgCode);
  smsc_log_debug(_logger, "LibSccpConnect::receive::: _ringBuf.readArray() returned buf=[%s]", utilx::hexdmp(inTp->packetBody,inTp->packetLen).c_str());

  changeProtocolState(CheckMessage(bufForMsgCode.msgCode));
  // TODO: FIX IT
//   try {
//     changeProtocolState(CheckMessage(bufForMsgCode.msgCode));
//   } catch (const io_dispatcher::ProtocolException& ex) {
//     send(libsua_messages::N_NOTICE_Message(libsua_messages::MErrorMessage::NO_SESSION_ESTABLISHED, inTp));
//     throw;
//   }
  inTp->packetType = bufForMsgCode.msgCode;

  smsc_log_debug(_logger, "LibSccpConnect::receive::: inTp->packetType=%d", inTp->packetType);
  if ( _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
    _ringBuf.readArray(_header, sizeof(_header));
    memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header, sizeof(_currentPacketSize));
    _currentPacketSize = ntohl(_currentPacketSize);
    if ( _currentPacketSize + sizeof(_header) > TP::MAX_PACKET_SIZE )
      throw smsc::util::Exception("LibSccpConnect::receive::: length [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize + sizeof(_header), TP::MAX_PACKET_SIZE);
  } else
    _currentPacketSize = 0;

  return inTp;
}

bool
LibSccpConnect::hasReadyTransportPacket()
{
  return _currentPacketSize > 0 && _currentPacketSize <= _ringBuf.getSizeOfAvailData();
}

void
LibSccpConnect::bufferInputTransportPackets()
{
  if ( _currentPacketSize == 0  ||
       _currentPacketSize > _ringBuf.getSizeOfAvailData() ) {
    try {
    _ringBuf.load(_socket->getInputStream()); // read as many bytes as possible into buffer from input stream
    } catch (corex::io::EOFException& ex) {
      try {
        changeProtocolState(TcpReleaseInd());
      } catch (...) {}
      throw;
    }
    if ( _currentPacketSize == 0 && _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
      _ringBuf.readArray(_header, sizeof(_header));
      memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header, sizeof(_currentPacketSize));
      _currentPacketSize = ntohl(_currentPacketSize);
      if ( _currentPacketSize + sizeof(_header) > TP::MAX_PACKET_SIZE )
        throw smsc::util::Exception("LibSccpConnect::bufferInputTransportPackets::: length [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize + sizeof(_header), TP::MAX_PACKET_SIZE);
    }
  }
}

void
LibSccpConnect::bind(const char* app_id)
{
  if ( _isPassiveSocket )
    throw smsc::util::Exception("LibSccpConnect::bind::: can't bind incoming session");

  libsccp::BindMessage bindMessage;
  bindMessage.setAppId(app_id);

  send(bindMessage);
}

void
LibSccpConnect::unbind()
{
  if ( _isPassiveSocket )
    throw smsc::util::Exception("LibSccpConnect::unbind::: can't unbind incoming session");

  libsccp::UnbindMessage unbindMessage;

  send(unbindMessage);
}

void
LibSccpConnect::tcpEstablish()
{
  if ( !_isPassiveSocket ) {
    changeProtocolState(TcpEstablishInd());
    _socket->connect();
  }
}

void
LibSccpConnect::tcpRelease()
{
  if ( !_isPassiveSocket ) {
    changeProtocolState(TcpReleaseInd());
    _socket->close();
  } else
    throw smsc::util::Exception("LibSccpConnect::tcpRelease::: can't release incoming session");
}

corex::io::network::Socket*
LibSccpConnect::_getSocket() const { return _socket; }

corex::io::OutputStream*
LibSccpConnect::getOutputStream() const
{
  return _socket->getOutputStream();
}

const io_dispatcher::ConnectAcceptor*
LibSccpConnect::getCreator() const
{
  return _creator;
}

}}}}
