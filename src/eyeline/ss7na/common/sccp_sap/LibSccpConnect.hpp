#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_LIBSCCPCONNECT_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_LIBSCCPCONNECT_HPP__

# include "logger/Logger.h"

# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/utilx/RingBuffer.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/sccp_sap/LibSccpConnectAcceptor.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class LibSccpConnect : public io_dispatcher::Link {
public:
  LibSccpConnect(const char* host, in_port_t port, const LinkId& link_id);
  LibSccpConnect(corex::io::network::TCPSocket* socket, const LibSccpConnectAcceptor* creator);

  virtual ~LibSccpConnect();
  //  virtual void send(const io_dispatcher::Message& message);

  virtual TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  void bind(const char* app_id);

  void unbind();

  void tcpEstablish();

  void tcpRelease();

  const io_dispatcher::ConnectAcceptor* getCreator() const;
protected:
  virtual corex::io::network::Socket* _getSocket() const;

  virtual corex::io::OutputStream* getOutputStream() const;
private:
  corex::io::network::TCPSocket* _socket;

  bool _isPassiveSocket;
  uint32_t _currentPacketSize;
  uint8_t _header[sizeof(uint32_t)];
  utilx::RingBuffer<TP::MAX_PACKET_SIZE> _ringBuf;
  smsc::logger::Logger* _logger;
  const LibSccpConnectAcceptor* _creator;
};

}}}}

#endif
