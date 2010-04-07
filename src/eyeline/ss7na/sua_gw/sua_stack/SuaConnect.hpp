#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_SUACONNECT_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_SUACONNECT_HPP__

# include <vector>
# include <netinet/in.h>
# include <logger/Logger.h>

# include "eyeline/corex/io/network/SCTPSocket.hpp"
# include "eyeline/utilx/RingBuffer.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/common/sig/SIGConnect.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/ProtocolStates.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class SuaConnect : public common::sig::SIGConnect {
public:
  SuaConnect(const std::vector<std::string> peer_addr_list, in_port_t port, const common::LinkId& link_id);

  SuaConnect(const std::vector<std::string> remote_addr_list, in_port_t remote_port,
             const std::vector<std::string> local_addr_list, in_port_t local_port,
             const common::LinkId& link_id);

  virtual common::TP* receive();

  virtual void send(const common::Message& message);

  virtual void up();

  virtual void down();

  virtual void activate();

  virtual void deactivate();
};

}}}}

#endif
