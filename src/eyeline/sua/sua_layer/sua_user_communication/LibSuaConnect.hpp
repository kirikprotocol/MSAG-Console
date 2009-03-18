#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECT_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECT_HPP__

# include <logger/Logger.h>

# include <eyeline/corex/io/network/TCPSocket.hpp>
# include <eyeline/utilx/RingBuffer.hpp>

# include <eyeline/sua/sua_layer/io_dispatcher/Link.hpp>
# include <eyeline/sua/communication/TP.hpp>
# include <eyeline/sua/communication/Message.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/sua_user_communication/LibSuaConnectAcceptor.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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

}}}}

#endif
