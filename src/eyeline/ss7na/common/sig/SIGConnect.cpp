#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

#include "eyeline/ss7na/common/AdaptationLayer_MsgCodesIndexer.hpp"

#include "SIGConnect.hpp"
#include "SctpEstablishInd.hpp"
#include "SctpReleaseInd.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sig {

//Fake message - need just for holding msg code value
class CheckMessage : public AdaptationLayer_Message {
public:
  CheckMessage(uint32_t msgCode) 
    : AdaptationLayer_Message(msgCode) {
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

SIGConnect::SIGConnect(io_dispatcher::ProtocolState* initial_state, const char* logger_name,
                       const std::vector<std::string> peer_addr_list, in_port_t port,
                       const LinkId& link_id)
  : Link(initial_state), _currentPacketSize(0),
    _peerAddrList(peer_addr_list), _r_port(port),
    _logger(smsc::logger::Logger::getInstance(logger_name))
{
  _socket = new corex::io::network::SCTPSocket(_peerAddrList[0], _r_port);
  if ( !_socket )
    throw smsc::util::SystemError("SIGConnect::SIGConnect::: can't create SCTPSocket");
  setLinkId(link_id);
}

SIGConnect::SIGConnect(io_dispatcher::ProtocolState* initial_state, const char* logger_name,
                       const std::vector<std::string> remote_addr_list, in_port_t remote_port,
                       const std::vector<std::string> local_addr_list, in_port_t local_port,
                       const LinkId& link_id)
  : Link(initial_state), _currentPacketSize(0),
    _peerAddrList(remote_addr_list), _localAddrList(local_addr_list),
    _r_port(remote_port), _l_port(local_port),
    _logger(smsc::logger::Logger::getInstance(logger_name))
{
  _socket = new corex::io::network::SCTPSocket(_peerAddrList[0], _r_port);
  if ( !_socket )
    throw smsc::util::SystemError("SIGConnect::SIGConnect::: can't create SCTPSocket");
  _socket->bindx(&local_addr_list[0], local_addr_list.size(), local_port);
  setLinkId(link_id);
}

void
SIGConnect::reinit()
{
  delete _socket;

  _socket = new corex::io::network::SCTPSocket(_peerAddrList[0], _r_port);
  if ( !_socket )
    throw smsc::util::SystemError("SIGConnect::reinit::: can't create SCTPSocket");
  _socket->bindx(&_localAddrList[0], _localAddrList.size(), _l_port);

  smsc_log_debug(_logger, "SIGConnect::reinit::: _socket reinited - _peerAddrList=%s, _r_port=%u, sockfd=%d",
                 _peerAddrList[0].c_str(), _r_port, _socket->getDescriptor());
  changeProtocolState(SctpReleaseInd());
}

SIGConnect::~SIGConnect()
{
  try {
    smsc_log_debug(_logger, "SIGConnect::~SIGConnect::: destroy object, fd=%d", _socket->getDescriptor());
    sctpRelease();
  } catch (...) {}
  delete _socket;
}

TP*
SIGConnect::receive()
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_PACKET>();
  TP* inTp = new (ptr_desc->allocated_memory) TP();

  smsc_log_debug(_logger, "SIGConnect::receive::: _currentPacketSize=%d, _ringBuf.getSizeOfAvailData()=%d, inTp=0x%p, ptr_desc->allocated_memory=0x%p", _currentPacketSize, _ringBuf.getSizeOfAvailData(), inTp, ptr_desc->allocated_memory);
  if ( _currentPacketSize - sizeof(_header) > _ringBuf.getSizeOfAvailData() ) return NULL;

  inTp->packetLen = _currentPacketSize;
  memcpy(inTp->packetBody, _header, sizeof(_header));
  _ringBuf.readArray(inTp->packetBody + sizeof(_header), _currentPacketSize - sizeof(_header));

  uint32_t msgCode = ((uint32_t)_header[2] << 8) | _header[3];

  changeProtocolState(CheckMessage(msgCode));

  inTp->packetType = AdaptationLayer_MsgCodesIndexer::getMessageIndex(msgCode);
  smsc_log_debug(_logger, "SIGConnect::receive::: _ringBuf.readArray() returned buf=[%s], inTp->packetType=%u", utilx::hexdmp(inTp->packetBody,_currentPacketSize).c_str(), inTp->packetType);
  if ( _ringBuf.getSizeOfAvailData() >= sizeof(_header) ) {
    _ringBuf.readArray(_header, sizeof(_header));
    memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header + AdaptationLayer_Message::MESSAGE_PREAMBLE_SIZE, AdaptationLayer_Message::MESSAGE_LENGTH_SIZE);
    _currentPacketSize = ntohl(_currentPacketSize);
    if ( _currentPacketSize > TP::MAX_PACKET_SIZE )
      throw smsc::util::Exception("SIGConnect::receive::: lenght [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize, TP::MAX_PACKET_SIZE);
  } else
    _currentPacketSize = 0;

  return inTp;
}

bool
SIGConnect::hasReadyTransportPacket()
{
  smsc_log_debug(_logger, "SIGConnect::hasReadyTransportPacket::: _currentPacketSize=[%d],_ringBuf.getSizeOfAvailData()=%d", _currentPacketSize, _ringBuf.getSizeOfAvailData());
  return _currentPacketSize > 0 &&
         (_currentPacketSize - sizeof(_header)) <= _ringBuf.getSizeOfAvailData();
}

void
SIGConnect::bufferInputTransportPackets()
{
  if ( _currentPacketSize == 0  ||
       (_currentPacketSize - sizeof(_header)) > _ringBuf.getSizeOfAvailData() ) {
    try {
    _ringBuf.load(_socket->getInputStream()); // read as many bytes as possible into buffer from input stream
    } catch (corex::io::EOFException& ex) {
      try {
        changeProtocolState(SctpReleaseInd());
      } catch (...) {}
      throw;
    }
    if ( _currentPacketSize == 0 && _ringBuf.getSizeOfAvailData() >= sizeof(_header)) {
      _ringBuf.readArray(_header, sizeof(_header));
      memcpy(reinterpret_cast<uint8_t*>(&_currentPacketSize), _header + AdaptationLayer_Message::MESSAGE_PREAMBLE_SIZE, AdaptationLayer_Message::MESSAGE_LENGTH_SIZE);
      _currentPacketSize = ntohl(_currentPacketSize);
      if ( _currentPacketSize > TP::MAX_PACKET_SIZE )
        throw smsc::util::Exception("SIGConnect::bufferInputTransportPackets::: lenght [=%d] of got message is greater than max allowable value [=%d]", _currentPacketSize, TP::MAX_PACKET_SIZE);
    }
  }
}

corex::io::network::Socket*
SIGConnect::_getSocket() const { return _socket; }

corex::io::OutputStream*
SIGConnect::getOutputStream() const
{
  return _socket->getOutputStream();
}

void
SIGConnect::sctpEstablish()
{
  // establish connect
  _socket->connect();
  smsc_log_info(_logger, "SIGConnect::sctpEstablish::: association '%s' has been established",
                _socket->toString().c_str());
  // if connection has been established , then make state transition
  changeProtocolState(SctpEstablishInd());
}

void
SIGConnect::sctpRelease()
{
  // release connect
  smsc_log_info(_logger, "SIGConnect::sctpRelease::: close association '%s'",
                _socket->toString().c_str());
  _socket->close();
  // if connection has been released , then make state transition
  changeProtocolState(SctpReleaseInd());
  smsc_log_debug(_logger, "SIGConnect::sctpRelease::: call to notifyLinkShutdownCompletion");
  if ( _subsystemListener ) _subsystemListener->notifyLinkShutdownCompletion();
}

}}}}
