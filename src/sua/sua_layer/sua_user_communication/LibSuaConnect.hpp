#ifndef SUA_USER_COMMUNICATION_LIBSUACONNECT_HPP_HEADER_INCLUDED_B87B618B
# define SUA_USER_COMMUNICATION_LIBSUACONNECT_HPP_HEADER_INCLUDED_B87B618B

# include <logger/Logger.h>
# include <sua/corex/io/network/TCPSocket.hpp>
# include <sua/utilx/RingBuffer.hpp>
# include <sua/sua_layer/io_dispatcher/Link.hpp>
# include <sua/communication/TP.hpp>
# include <sua/communication/Message.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/sua_layer/sua_user_communication/LibSuaConnectAcceptor.hpp>

namespace sua_user_communication {

class LibSuaConnect : public io_dispatcher::Link {
public:
  //##Documentation
  //## Параметры соединения определяются конструктором.
  LibSuaConnect(const char* host, in_port_t port, const communication::LinkId& linkId);
  LibSuaConnect(corex::io::network::TCPSocket* socket, const LibSuaConnectAcceptor* creator);

  virtual ~LibSuaConnect();
  //  virtual void send(const io_dispatcher::Message& message);

  virtual communication::TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  void bind(const char* appId);

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
  utilx::RingBuffer<communication::TP::MAX_PACKET_SIZE> _ringBuf;
  smsc::logger::Logger* _logger;
  const LibSuaConnectAcceptor* _creator;
};

} // namespace sua_user_communication

#endif /* SUA_USER_COMMUNICATION_LIBSUACONNECT_HPP_HEADER_INCLUDED_B87B618B */
