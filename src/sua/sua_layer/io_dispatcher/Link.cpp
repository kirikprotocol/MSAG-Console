#include <sua/sua_layer/io_dispatcher/Link.hpp>
#include <sua/utilx/Exception.hpp>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace io_dispatcher {

Link::Link(ProtocolState* initialState)
  : _whole(NULL), _protocolStateCtrl(initialState), _queue(100), _flag(false) {}

Link::~Link() {}

communication::TP*
Link::receive()
{
  return NULL;
}

bool
Link::hasReadyTransportPacket()
{
  return false;
}

void
Link::bufferInputTransportPackets()
{
}

corex::io::network::Socket*
Link::getSocket()
{
  return _getSocket();
}

void
Link::send(const communication::Message& message)
{
  changeProtocolState(message);

  communication::TP* transportPacket = new communication::TP();

  message.serialize(transportPacket);

  genericSend(getOutputStream(), transportPacket);
}

void
Link::genericSend(corex::io::OutputStream* oStream, const communication::TP* packet)
{
  {
    smsc::core::synchronization::MutexGuard lock(_lock);
    if ( _flag )  {
      if ( !_queue.enqueue(packet) ) {
        delete packet;
        throw utilx::CongestionException("Link::genericSend::: output queue is full");
      }
      return;
    } else
      _flag = true;
  }
  const communication::TP* packetToSend = packet;
  do {
    sendToStream(oStream, packetToSend);
    delete packetToSend;
    {
      smsc::core::synchronization::MutexGuard lock(_lock);
      if ( _queue.isEmpty() ) {
        _flag = false; break;
      }
      _queue.dequeue(packetToSend);
    }
  } while (true);
}

void
Link::sendToStream(corex::io::OutputStream* oStream, const communication::TP* packetToSend)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("io_dsptch");
  smsc_log_info(logger, "Link::sendToStream::: next buffer will be written to oStream [%s]", hexdmp(packetToSend->packetBody, packetToSend->packetLen).c_str());
  const uint8_t* bufPositionPtr = packetToSend->packetBody;
  size_t leftToSend = packetToSend->packetLen;
  while (leftToSend > 0) {
    ssize_t sz = oStream->write(bufPositionPtr, leftToSend, packetToSend->pkt_sctp_props.streamNo, packetToSend->pkt_sctp_props.orderingTransfer);
    leftToSend -= sz; bufPositionPtr += sz;
  }
}

void
Link::setWhole(LinkSet* whole)
{
  _whole = whole;
}

LinkSet*
Link::getWhole() const
{
  return _whole;
}

void
Link::changeProtocolState(const communication::Message& message)
{
  smsc::core::synchronization::MutexGuard lock(_protStateCtrlLock);
  _protocolStateCtrl.doStateTransition(message);
}

void
Link::changeProtocolState(const IndicationPrimitive& indication)
{
  smsc::core::synchronization::MutexGuard lock(_protStateCtrlLock);
  _protocolStateCtrl.doStateTransition(indication);
}

}
