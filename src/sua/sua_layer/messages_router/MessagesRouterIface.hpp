#ifndef __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERIFACE_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERIFACE_HPP__ 1

# include <sua/sua_layer/messages_router/RoutingKey.hpp>
# include <sua/communication/LinkId.hpp>

namespace messages_router {

class MessagesRouterIface {
public:
  virtual ~MessagesRouterIface() {}

  virtual communication::LinkId getOutLink(const RoutingKey& messageRoutingInfo) = 0;
  virtual communication::LinkId getOutLink(const RoutingKey& messageRoutingInfo, bool* callTerminationIndicator) = 0;

};

}

#endif
