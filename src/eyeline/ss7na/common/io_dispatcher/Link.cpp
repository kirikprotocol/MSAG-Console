#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Link.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

Link::Link(ProtocolState* initial_state)
  : _whole(NULL), _protocolStateCtrl(initial_state), _queue(100), _flag(false),
    _logger(smsc::logger::Logger::getInstance("io_dsptch")) {}

Link::~Link() {}

TP*
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
Link::getSocket() const
{
  return _getSocket();
}

void
Link::send(const Message& message)
{
  changeProtocolState(message);

  TP* transportPacket = new TP();

  message.serialize(transportPacket);

  genericSend(getOutputStream(), transportPacket);
}

void
Link::genericSend(corex::io::OutputStream* o_stream, const TP* packet)
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

  const TP* packetToSend = packet;
  try {
    do {
      sendToStream(o_stream, packetToSend);
      delete packetToSend; packetToSend = NULL;
      {
        smsc::core::synchronization::MutexGuard lock(_lock);
        if ( _queue.isEmpty() ) {
          _flag = false; break;
        }
        _queue.dequeue(packetToSend);
      }
    } while (true);
  } catch (...) {
    delete packetToSend;
    throw;
  }
}

void
Link::sendToStream(corex::io::OutputStream* o_stream, const TP* packet_to_send)
{
  smsc_log_debug(_logger, "Link::sendToStream::: next buffer will be written to oStream [%s]", utilx::hexdmp(packet_to_send->packetBody, packet_to_send->packetLen).c_str());
  const uint8_t* bufPositionPtr = packet_to_send->packetBody;
  size_t leftToSend = packet_to_send->packetLen;
  while (leftToSend > 0) {
    ssize_t sz = o_stream->write(bufPositionPtr, leftToSend, packet_to_send->pkt_sctp_props.streamNo, packet_to_send->pkt_sctp_props.orderingTransfer);
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
Link::changeProtocolState(const Message& message)
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

}}}}
