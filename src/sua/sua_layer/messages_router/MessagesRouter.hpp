#ifndef __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTER_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTER_HPP__ 1

# include <logger/Logger.h>
# include <sua/utilx/PrefixTree.hpp>
# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/messages_router/MessagesRouterIface.hpp>

namespace messages_router {

class MessagesRouter : public utilx::Singleton<MessagesRouter>,
                       public MessagesRouterIface {
public:
  MessagesRouter();
  virtual communication::LinkId getOutLink(const RoutingKey& messageRoutingInfo);
  virtual communication::LinkId getOutLink(const RoutingKey& messageRoutingInfo, bool* callTerminationIndicator);

  // registry routing entry for incoming messages (from SGP ==> to USER)
  void addIncomingRouteEntry(const std::string& destinationGtMask, const communication::LinkId& linkToUser);
  void addIncomingRouteEntry(const std::string& destinationGtMask, uint8_t destinationSSN, const communication::LinkId& linkToUser);
  // registry routing entry for outcoming messages (from USER ==> to SGP)
  void addOutcomingRouteEntry(const std::string& destinationGtMask, const communication::LinkId& linkToSGP);
  void addOutcomingRouteEntry(const std::string& destinationGtMask, uint8_t destinationSSN, const communication::LinkId& linkToSGP);
private:
  smsc::logger::Logger* _logger;

  static const int RTABLE_SIZE = 256;
  static const int DEFAULT_RTABLE_ENTRY = 255;

  typedef utilx::PrefixTree<communication::LinkId> rtable_t;

  rtable_t* _outRoutingTable[RTABLE_SIZE];
  rtable_t* _inRoutingTable[RTABLE_SIZE];
};

}

#endif
