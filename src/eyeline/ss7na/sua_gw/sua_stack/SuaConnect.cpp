#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/PreallocatedMemoryManager.hpp"

#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/AdaptationLayer_MsgCodesIndexer.hpp"
#include "eyeline/ss7na/sua_gw/types.hpp"

#include "messages/DownMessage.hpp"
#include "messages/UPMessage.hpp"
#include "messages/ActiveMessage.hpp"
#include "messages/InactiveMessage.hpp"
#include "messages/CLDTMessage.hpp"

#include "RCRegistry.hpp"
#include "SuaConnect.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

SuaConnect::SuaConnect(const std::vector<std::string> peer_addr_list, in_port_t port,
                       const common::LinkId& link_id)
  : common::sig::SIGConnect(SUA_State_NoConnection::getInstance(),"sua_stack",
                            peer_addr_list, port, link_id)
{}

SuaConnect::SuaConnect(const std::vector<std::string> remote_addr_list, in_port_t remote_port,
                       const std::vector<std::string> local_addr_list, in_port_t local_port,
                       const common::LinkId& link_id)
  : common::sig::SIGConnect(SUA_State_NoConnection::getInstance(), "sua_stack",
                            remote_addr_list, remote_port,
                            local_addr_list, local_port, link_id)
{}

common::TP*
SuaConnect::receive()
{
  common::TP* tp = SIGConnect::receive();
  tp->protocolClass = PROTOCOL_SUA;
  return tp;
}

void
SuaConnect::send(const common::Message& message)
{
  const common::AdaptationLayer_Message& suaMessage = static_cast<const common::AdaptationLayer_Message&>(message);
  unsigned int msgClass = suaMessage.getMessageClass();
  if ( (msgClass == common::AdaptationLayer_Message::SUA_MANAGEMENT_MESSAGES /* ERROR or NOTIFY */||
        msgClass == common::AdaptationLayer_Message::SUA_SIGNALING_NETWORK_MANAGEMENT_MESSAGES ||
        msgClass == common::AdaptationLayer_Message::CONNECTIONLESS_MESSAGES ) &&
       suaMessage.isSetRoutingContext() ) {
    suaMessage.updateRoutingContext(RCRegistry::getInstance().getRoutingContext(getLinkId()));
  }
  Link::send(suaMessage);
}

void
SuaConnect::up()
{
  messages::UPMessage upMessage;
  send(upMessage);
}

void
SuaConnect::down()
{
  messages::DownMessage downMessage;
  send(downMessage);
}

void
SuaConnect::activate()
{
  messages::ActiveMessage activeMessage;
  send(activeMessage);
}

void
SuaConnect::deactivate()
{
  messages::InactiveMessage inactiveMessage;
  send(inactiveMessage);
}

}}}}
