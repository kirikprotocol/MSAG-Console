#ifndef SUA_STACK_SUACONNECT_HPP_HEADER_INCLUDED_B87B0721
# define SUA_STACK_SUACONNECT_HPP_HEADER_INCLUDED_B87B0721

# include <vector>
# include <netinet/in.h>
# include <logger/Logger.h>
# include <sua/corex/io/network/SCTPSocket.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/TP.hpp>
# include <sua/communication/sua_messages/SUAMessage.hpp>
# include <sua/sua_layer/io_dispatcher/Link.hpp>
# include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
# include <sua/sua_layer/sua_stack/ProtocolStates.hpp>
# include <sua/utilx/RingBuffer.hpp>

namespace sua_stack {

class SuaStackSubsystem;

class SuaConnect : public io_dispatcher::Link {
public:
  //##Documentation
  //## Параметры соединения оппеделяются конструктором.
  SuaConnect(const std::vector<std::string> peerAddrList, in_port_t port, const communication::LinkId& linkId);

  SuaConnect(const std::vector<std::string> remoteAddrList, in_port_t remotePort,
             const std::vector<std::string> localAddrList, in_port_t localPort,
             const communication::LinkId& linkId);

  virtual ~SuaConnect();

  virtual communication::TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  void up();

  void down();

  void activate();

  void deactivate();

  void sctpEstablish();

  void sctpRelease();

  void setListener(SuaStackSubsystem* subsystem);

protected:
  virtual corex::io::network::Socket* _getSocket() const;

  virtual corex::io::OutputStream* getOutputStream() const;
private:
  corex::io::network::SCTPSocket* _socket;

  uint8_t _header[sua_messages::SUAMessage::HEADER_SIZE];
  uint32_t _currentPacketSize;
  utilx::RingBuffer<communication::TP::MAX_PACKET_SIZE> _ringBuf;

  std::vector<std::string> _peerAddrList;
  in_port_t _r_port;

  SuaStackSubsystem* _subsystemListener;
  smsc::logger::Logger* _logger;
};

} // namespace sua_stack



#endif /* SUACONNECT_HPP_HEADER_INCLUDED_B87B0721 */
