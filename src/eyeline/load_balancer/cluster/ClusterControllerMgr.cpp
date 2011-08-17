#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <sys/time.h>

#include "ClusterControllerMgr.hpp"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/protocol/Magics.hpp"
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/clustercontroller/protocol/Magics.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

ClusterControllerMgr::ClusterControllerMgr(const std::string& peer_host,
                                           in_port_t peer_port,
                                           unsigned connect_timeout,
                                           unsigned connect_attempt_period)
: _logger(smsc::logger::Logger::getInstance("cluster")),
  _shutdownInProgress(false), _connected(false),
  _peerHost(peer_host), _peerPort(peer_port),
  _connectTimeout(connect_timeout),
  _connectAttemptPeriod(connect_attempt_period),
  _protocolMessagesHandler(*this, _protocolMessagesDispatcher)
{
  _socket = new corex::io::network::TCPSocket(_peerHost, _peerPort);
  _protocolMessagesDispatcher.assignHandler(&_protocolMessagesHandler);
}

ClusterControllerMgr::~ClusterControllerMgr()
{
  WaitFor();
  delete _socket;
}

int
ClusterControllerMgr::Execute()
{
  io_subsystem::Packet packet;

  try {
    while ( !_shutdownInProgress ) {
      try {
        while ( !_connected )
          tryConnect();

        if ( _shutdownInProgress ) return 0;

        readPacket(&packet);
        processCommand(packet);
      } catch (corex::io::ConnectionFailedException& ex) {
        smsc_log_error(_logger, "ClusterControllerMgr::Execute::: connection failed to cluster controller (host=%s,port=%d), sleep %d seconds and try again",
                       _peerHost.c_str(), _peerPort, _connectAttemptPeriod);
        waitForPeriod(_connectAttemptPeriod);
        delete _socket;
        _socket = new corex::io::network::TCPSocket(_peerHost, _peerPort);
      } catch (std::exception& ex) {
        smsc_log_error(_logger, "ClusterControllerMgr::Execute::: caught exception [%s]",
                       ex.what());
        delete _socket;
        sleep(1);
        _socket = new corex::io::network::TCPSocket(_peerHost, _peerPort);
        _connected=false;
      }
    }
  } catch(std::exception& ex) {
    smsc_log_error(_logger, "ClusterControllerMgr::Execute::: caught unexpected exception [%s]",
                   ex.what());
    return 1;
  }
  return 0;
}

void
ClusterControllerMgr::startup()
{
  smsc_log_debug(_logger, "ClusterControllerMgr::startup::: try start thread");
  Start();
}

void
ClusterControllerMgr::shutdown()
{
  smsc_log_debug(_logger, "ClusterControllerMgr::startup::: try shutdown thread");
  _shutdownInProgress = true;
  _socket->close();
}

void
ClusterControllerMgr::tryConnect()
{
  if ( _shutdownInProgress ) return;

  smsc_log_debug(_logger, "ClusterControllerMgr::tryConnect::: try connect to cluster controller (host=%s,port=%d)",
                 _peerHost.c_str(), _peerPort);
  _socket->connect(_connectTimeout);
  smsc_log_info(_logger, "ClusterControllerMgr::tryConnect::: connect to cluster controller (host=%s,port=%d) has been established",
                _peerHost.c_str(), _peerPort);

  try {
    messages::RegisterAsLoadBalancer registerMessage;
    registerMessage.setMagic(clustercontroller::protocol::pmLoadBalancer);

    protogen::framework::SerializerBuffer bufferForSend(64);
    _protocolMessagesDispatcher.encodeMessage(registerMessage, &bufferForSend);
    smsc_log_info(_logger, "ClusterControllerMgr::tryConnect::: send registration message [%s] to connect [%s]",
                  registerMessage.toString().c_str(), _socket->toString().c_str());
    writeBuffer(bufferForSend.getBuffer(), bufferForSend.getDataWritten());

    _connected = true;
  } catch (std::exception& ex) {
    throw corex::io::ConnectionFailedException(ex.what());
  }
}

void
ClusterControllerMgr::writeBuffer(const char* bufferForWrite, size_t bytesToWrite)
{
  writen(_socket->getOutputStream(), reinterpret_cast<const unsigned char*>(bufferForWrite),
         bytesToWrite);
}

void
ClusterControllerMgr::readPacket(io_subsystem::Packet* packet)
{
  union {
    uint8_t data[sizeof(uint32_t)];
    uint32_t len;
  } lenbuf;

  readn(_socket->getInputStream(), lenbuf.data, sizeof(lenbuf.data));
  packet->packet_data_len = ntohl(lenbuf.len);
  readn(_socket->getInputStream(), packet->packet_data, packet->packet_data_len);
}

void
ClusterControllerMgr::processCommand(const io_subsystem::Packet& packet)
{
  _protocolMessagesDispatcher.decodeAndHandleMessage(reinterpret_cast<const char*>(packet.packet_data),
                                                     packet.packet_data_len);
}

void
ClusterControllerMgr::waitForPeriod(unsigned period)
{
  struct timeval timeout;
  timeout.tv_sec = period; timeout.tv_usec = 0;
  if ( ::select(0, NULL, NULL, NULL, &timeout) < 0 )
    throw smsc::util::SystemError("ClusterControllerMgr::waitPeriod::: call to select() failed");
}

void
ClusterControllerMgr::readn(corex::io::InputStream* istream,
                            uint8_t* databuf, size_t databuf_len)
{
  uint8_t* ptr = databuf;
  while ( databuf_len ) {
    ssize_t bytesWasRead = istream->read(ptr, databuf_len);
    ptr += bytesWasRead; databuf_len -= bytesWasRead;
  }
}

void
ClusterControllerMgr::writen(corex::io::OutputStream* ostream,
                             const uint8_t* databuf, size_t databuf_len)
{
  if ( _shutdownInProgress ) return;

  const uint8_t* ptr = databuf;
  while ( databuf_len ) {
    ssize_t bytesWasWritten = ostream->write(ptr, databuf_len);
    ptr += bytesWasWritten; databuf_len -= bytesWasWritten;
  }
}

}}}
