#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPCONNECTION_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPCONNECTION_HPP__

# include <sys/types.h>
# include <string>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/utilx/prot_fsm/TCPIndicationPrimitive.hpp"
# include "eyeline/utilx/prot_fsm/ProtocolStateController.hpp"

# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPPacketFormatter.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPProtocolStateController.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPPConnection : public io_subsystem::Link {
public:
  explicit SMPPConnection(corex::io::network::TCPSocket* socket);

  SMPPConnection(const std::string& peer_host,
                 in_port_t peer_port,
                 unsigned int connect_timeout,
                 unsigned int bind_resp_wait_timeout,
                 unsigned int unbind_resp_wait_timeout);

  virtual ~SMPPConnection();

  virtual void establish();
  virtual void close();

  virtual const std::string& getPeerInfoString() const;
  virtual Link* createNewOutLink() const;

  virtual io_subsystem::Packet* receive();
  virtual io_subsystem::LinkId send(const io_subsystem::Message& message);

  virtual corex::io::IOObject& getIOObject() { return *_socket; }

protected:
  io_subsystem::Message::message_code_t getMsgCode(const io_subsystem::Packet& packet) const;
  void checkProtocolState(io_subsystem::Message::message_code_t msgCode) /*synchronized*/;
  void checkProtocolState(utilx::prot_fsm::TCPIndicationPrimitive& networkIndication) /*synchronized*/;

private:
  smsc::logger::Logger* _logger;
  corex::io::network::TCPSocket* _socket;
  void* _memForSocket;
  std::string _peerHost, _peerInfoString;
  in_port_t _peerPort;
  bool _isConnectionIncoming;

  SMPPPacketFormatter _packetFormatter;

  smsc::core::synchronization::Mutex _protStateCtrlLock;
  SMPPProtocolStateController _protocolStateCtrl;
};

}}}}

#endif
