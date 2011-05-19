#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"

#include "eyeline/corex/io/IOExceptions.hpp"
#include "eyeline/utilx/prot_fsm/TcpReleaseInd.hpp"
#include "eyeline/utilx/prot_fsm/TcpEstablishInd.hpp"

#include "eyeline/load_balancer/io_subsystem/Exception.hpp"
#include "eyeline/load_balancer/io_subsystem/PacketWriter.hpp"

#include "SMPPConnection.hpp"
#include "ProtocolStates.hpp"
#include "EnquireLink.hpp"

#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

SMPPConnection::SMPPConnection(corex::io::network::TCPSocket* socket)
  : _logger(smsc::logger::Logger::getInstance("smpp")),
    _socket(socket), _memForSocket(NULL), _peerPort(0), _isConnectionIncoming(true),
    _protocolStateCtrl(SMPP_Connected::getInstance())
{
  setLinkId(io_subsystem::LinkId(_socket->toString()));
  _socket->setId(socket->toString());
  _peerInfoString = "connection is incoming";
}

SMPPConnection::SMPPConnection(unsigned int link_index,
                               const std::string& peer_host,
                               in_port_t peer_port,
                               unsigned int connect_timeout,
                               unsigned int bind_resp_wait_timeout,
                               unsigned int unbind_resp_wait_timeout)
  : io_subsystem::Link(link_index, connect_timeout, bind_resp_wait_timeout, unbind_resp_wait_timeout),
    _logger(smsc::logger::Logger::getInstance("smpp")),
    _socket(NULL), _peerHost(peer_host), _peerPort(peer_port),
    _isConnectionIncoming(false), _protocolStateCtrl(SMPP_NotConnected::getInstance())
{
  if ( !unbind_resp_wait_timeout )
    throw smsc::util::Exception("SMPPConnection::SMPPConnection::: unbind_resp_wait_timeout value=0");
  _memForSocket = operator new (sizeof(corex::io::network::TCPSocket));
  char peerPortStr[32];
  sprintf(peerPortStr, ",%d", peer_port);
  _peerInfoString = peer_host + peerPortStr;
}

SMPPConnection::~SMPPConnection()
{
  if ( _socket ) {
    smsc_log_debug(_logger, "SMPPConnection::~SMPPConnection::: remove socket [%s]",
                   _socket->toString().c_str());
    delete _socket;
  } else if ( _memForSocket )
    operator delete (_memForSocket);
}

void
SMPPConnection::establish()
{
  if ( _isConnectionIncoming )
    throw smsc::util::Exception("SMPPConnection::establish::: this connection is incoming");
  if ( _socket )
    throw smsc::util::Exception("SMPPConnection::establish::: connection has been already established");

  corex::io::network::TCPSocket* socket =
    new (_memForSocket) corex::io::network::TCPSocket(_peerHost, _peerPort);
  try {
    socket->connect(getConnectTimeout());
  } catch (...) {
    socket->~TCPSocket();
    throw;
  }
  _socket = socket;
  utilx::prot_fsm::TcpEstablishInd state;
  checkProtocolState(state);
  setLinkId(io_subsystem::LinkId(_socket->toString()));
  _socket->setId(_socket->toString());
}

void
SMPPConnection::close()
{
  try {
    utilx::prot_fsm::TcpReleaseInd state;
    checkProtocolState(state);
  } catch (utilx::ProtocolException& ex) {}
  _socket->close();
  resetLinkId();
}

const std::string&
SMPPConnection::getPeerInfoString() const
{
  return _peerInfoString;
}

io_subsystem::Packet*
SMPPConnection::receive()
{
  try {
    std::auto_ptr<io_subsystem::Packet> packet;
    packet.reset(_packetFormatter.receive(getIOObject().getInputStream()));

    if ( packet.get() )
      checkProtocolState(getMsgCode(*packet));

    return packet.release();
  } catch (corex::io::EOFException& ex) {
    utilx::prot_fsm::TcpReleaseInd state;
    checkProtocolState(state);
    throw;
  }
}

io_subsystem::LinkId
SMPPConnection::send(const io_subsystem::Message& message)
{
  smsc_log_info(_logger, "SMPPConnection::send::: Try send message [%s] over connection with id='%s'",
                message.toString().c_str(), getLinkId().toString().c_str());
  checkProtocolState(message.getMsgCode());

  io_subsystem::Packet* packet = _packetFormatter.prepareToSend(message);
  smsc_log_debug(_logger, "SMPPConnection::send::: packet dump='%s'", utilx::hexdmp(packet->packet_data, packet->packet_data_len).c_str());
  _packetWriter->send(packet, getLinkId());

  return getLinkId();
}

io_subsystem::Message::message_code_t
SMPPConnection::getMsgCode(const io_subsystem::Packet& packet) const
{
  return packet.packet_type;
}

void
SMPPConnection::checkProtocolState(io_subsystem::Message::message_code_t msgCode)
{
  smsc::core::synchronization::MutexGuard lock(_protStateCtrlLock);
  _protocolStateCtrl.doStateTransition(SMPPMessage(msgCode));
}

void
SMPPConnection::checkProtocolState(utilx::prot_fsm::TCPIndicationPrimitive& networkIndication)
{
  smsc::core::synchronization::MutexGuard lock(_protStateCtrlLock);
  _protocolStateCtrl.doStateTransition(networkIndication);
}


io_subsystem::Link*
SMPPConnection::createNewOutLink() const
{
  if ( _isConnectionIncoming )
    throw smsc::util::Exception("SMPPConnection::duplicateOutLink::: this connection is incoming");


  return new SMPPConnection(getIndex(), _peerHost, _peerPort, getConnectTimeout(),
                            getBindRespWaitTimeout(), getUnbindRespWaitTimeout());
}

}}}}
