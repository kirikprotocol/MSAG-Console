#include <sua/communication/sua_messages/SUAMessage.hpp>
#include <sua/utilx/PreallocatedMemoryManager.hpp>
#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>

#include <sua/communication/sua_messages/DownMessage.hpp>
#include <sua/communication/sua_messages/UPMessage.hpp>
#include <sua/communication/sua_messages/ActiveMessage.hpp>
#include <sua/communication/sua_messages/InactiveMessage.hpp>

#include "SuaConnect.hpp"
#include "SctpEstablishInd.hpp"
#include "SctpReleaseInd.hpp"
#include "SuaStackSubsystem.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace sua_stack {

//Fake message - need just for holding msg code value
class CheckMessage : public sua_messages::SUAMessage {
public:
  CheckMessage(uint32_t msgCode) 
    : SUAMessage(msgCode) {
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

SuaConnect::SuaConnect(const std::vector<std::string> peerAddrList, in_port_t port, const communication::LinkId& linkId)
  : Link(sua_stack::SUA_State_NoConnection::getInstance()), _currentPacketSize(0),
    _peerAddrList(peerAddrList), _r_port(port), _subsystemListener(NULL),
    _logger(smsc::logger::Logger::getInstance("sua_stack"))
{
  _socket = new corex::io::network::SCTPSocket(peerAddrList[0], port);
  if ( !_socket )
    throw smsc::util::SystemError("SuaConnect::SuaConnect::: can't create SCTPSocket");
  setLinkId(linkId);
}

SuaConnect::~SuaConnect()
{
  try {
    smsc_log_debug(_logger, "SuaConnect::~SuaConnect::: destroy object");
    sctpRelease();
  } catch (...) {}
  delete _socket;
}

communication::TP*
SuaConnect::receive()
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_PACKET>();
  communication::TP* inTp = new (ptr_desc->allocated_memory) communication::TP();

  smsc_log_debug(_logger, "SuaConnect::receive::: _currentPacketSize=%d, _ringBuf.getSizeOfAvailData()=%d, inTp=0x%p,ptr_desc->allocated_memory=0x%p", _currentPacketSize, _ringBuf.getSizeOfAvailData(), inTp, ptr_desc->allocated_memory);
  if ( _currentPacketSize - sizeof(_header) > _ringBuf.getSizeOfAvailData() ) return NULL;

  inTp->packetLen = _currentPacketSize;
  memcpy(inTp->packetBody, _header, sizeof(_header));
  _ringBuf.readArray(inTp->packetBody + sizeof(_header), _currentPacketSize - sizeof(_header));

  uint32_t msgCode = ((uint32_t)_header[2] << 8) | _header[3];

  changeProtocolState(CheckMessage(msgCode));

  inTp->packetType = sua_messages::SUAMessage::getMessageIndex(msgCode);
  smsc_log_debug(_logger, "SuaConnect::receive::: _ringBuf.readArray() returned buf=[%s]", hexdmp(inTp->packetBody,_currentPacketSize).c_str());
  smsc_log_debug(_logger, "SuaConnect::receive::: inTp->packetType=%d", inTp->packetType);
  if ( _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
    _ringBuf.readArray(_header, sizeof(_header));
    memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header + sua_messages::SUAMessage::MESSAGE_PREAMBLE_SIZE, sua_messages::SUAMessage::MESSAGE_LENGTH_SIZE);
    _currentPacketSize = ntohl(_currentPacketSize);
    if ( _currentPacketSize > communication::TP::MAX_PACKET_SIZE )
      throw smsc::util::Exception("SuaConnect::receive::: lenght [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize, communication::TP::MAX_PACKET_SIZE);
  } else
    _currentPacketSize = 0;

  return inTp;
}

bool
SuaConnect::hasReadyTransportPacket()
{
  smsc_log_debug(_logger, "SuaConnect::hasReadyTransportPacket::: _currentPacketSize=[%d],_ringBuf.getSizeOfAvailData()=%d", _currentPacketSize, _ringBuf.getSizeOfAvailData());
  return _currentPacketSize > 0 && (_currentPacketSize - sizeof(_header)) <= _ringBuf.getSizeOfAvailData();
}

void
SuaConnect::bufferInputTransportPackets()
{
  if ( _currentPacketSize == 0  ||
       (_currentPacketSize - sizeof(_header)) > _ringBuf.getSizeOfAvailData() ) {
    _ringBuf.load(_socket->getInputStream()); // read as many bytes as possible into buffer from input stream
    if ( _currentPacketSize == 0 && _ringBuf.getSizeOfAvailData() >= sizeof(_header)) {
      _ringBuf.readArray(_header, sizeof(_header));
      memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header + sua_messages::SUAMessage::MESSAGE_PREAMBLE_SIZE, sua_messages::SUAMessage::MESSAGE_LENGTH_SIZE);
      _currentPacketSize = ntohl(_currentPacketSize);
      if ( _currentPacketSize > communication::TP::MAX_PACKET_SIZE )
        throw smsc::util::Exception("SuaConnect::bufferInputTransportPackets::: lenght [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize, communication::TP::MAX_PACKET_SIZE);
    }
  }
}

void
SuaConnect::up()
{
  sua_messages::UPMessage upMessage;
  send(upMessage);
}

void
SuaConnect::down()
{
  sua_messages::DownMessage downMessage;
  send(downMessage);
}

void
SuaConnect::activate()
{
  sua_messages::ActiveMessage activeMessage;
  send(activeMessage);
}

void
SuaConnect::deactivate()
{
  sua_messages::InactiveMessage inactiveMessage;
  send(inactiveMessage);
}

void
SuaConnect::sctpEstablish()
{
  // establish connect
  _socket->connect();
  // if connection has been established , then make state transition
  changeProtocolState(SctpEstablishInd());
}

void
SuaConnect::sctpRelease()
{
  // release connect
  smsc_log_debug(_logger, "SuaConnect::sctpRelease::: close association");
  _socket->close();
  // if connection has been released , then make state transition
  changeProtocolState(SctpReleaseInd());
  smsc_log_debug(_logger, "SuaConnect::sctpRelease::: call to notifyLinkShutdownCompletion");
  if ( _subsystemListener ) _subsystemListener->notifyLinkShutdownCompletion();
}

corex::io::network::Socket*
SuaConnect::_getSocket() const { return _socket; }

corex::io::OutputStream*
SuaConnect::getOutputStream() const
{
  return _socket->getOutputStream();
}

void
SuaConnect::setListener(SuaStackSubsystem* subsystem)
{
  _subsystemListener = subsystem;
}

}
