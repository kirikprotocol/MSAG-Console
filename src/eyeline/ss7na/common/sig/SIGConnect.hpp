#ifndef __EYELINE_SS7NA_COMMON_SIG_SIGCONNECT_HPP__
# define __EYELINE_SS7NA_COMMON_SIG_SIGCONNECT_HPP__

# include <vector>
# include <netinet/in.h>

# include "logger/Logger.h"

# include "eyeline/corex/io/network/SCTPSocket.hpp"
# include "eyeline/utilx/RingBuffer.hpp"

# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sig {

class SIGConnect : public io_dispatcher::Link {
public:
  SIGConnect(io_dispatcher::ProtocolState* initial_state, const char* logger_name,
             const std::vector<std::string> peer_addr_list,
             in_port_t port, const LinkId& link_id);

  SIGConnect(io_dispatcher::ProtocolState* initial_state, const char* logger_name,
             const std::vector<std::string> remote_addr_list, in_port_t remote_port,
             const std::vector<std::string> local_addr_list, in_port_t local_port,
             const LinkId& link_id);

  virtual ~SIGConnect();

  virtual TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  virtual void up() = 0;

  virtual void down() = 0;

  virtual void activate() = 0;

  virtual void deactivate() = 0;

  void reinit();

  void sctpEstablish();
  void sctpRelease();

  void setListener(ApplicationSubsystem* subsystem) { _subsystemListener = subsystem; }

protected:
  virtual corex::io::network::Socket* _getSocket() const;

  virtual corex::io::OutputStream* getOutputStream() const;

  corex::io::network::SCTPSocket* _socket;

  uint8_t _header[AdaptationLayer_Message::HEADER_SIZE];
  uint32_t _currentPacketSize;
  utilx::RingBuffer<TP::MAX_PACKET_SIZE> _ringBuf;

  std::vector<std::string> _peerAddrList, _localAddrList;
  in_port_t _r_port, _l_port;

  ApplicationSubsystem* _subsystemListener;

  smsc::logger::Logger* _logger;
};

}}}}

#endif
