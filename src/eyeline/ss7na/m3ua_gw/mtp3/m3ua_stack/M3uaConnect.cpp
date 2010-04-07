#include "messages/DownMessage.hpp"
#include "messages/UPMessage.hpp"
#include "messages/ActiveMessage.hpp"
#include "messages/InactiveMessage.hpp"

#include "M3uaConnect.hpp"
#include "ProtocolStates.hpp"
#include "eyeline/ss7na/m3ua_gw/point_status_mgmt/PointStatusTable.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/AdjacentDPCRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

M3uaConnect::M3uaConnect(const std::vector<std::string> peer_addr_list, in_port_t port,
                         const common::LinkId& link_id)
  : common::sig::SIGConnect(M3UA_State_NoConnection::getInstance(),"m3ua_stack",
                            peer_addr_list, port, link_id)
{
  std::string remoteIfaces;
  for (std::vector<std::string>::const_iterator iter = peer_addr_list.begin(), end_iter = peer_addr_list.end();
      iter != end_iter; ++iter) {
    remoteIfaces += *iter + ",";
  }
  char portStr[32];
  sprintf(portStr, "%u", port);
  std::string::size_type idx = remoteIfaces.rfind(',');
  remoteIfaces.insert(idx, portStr);

  smsc_log_info(_logger, "M3uaConnect::M3uaConnect::: connection to %s, link_id=%s",
                remoteIfaces.c_str(), link_id.getValue().c_str());
}

M3uaConnect::M3uaConnect(const std::vector<std::string> remote_addr_list, in_port_t remote_port,
                         const std::vector<std::string> local_addr_list, in_port_t local_port,
                         const common::LinkId& link_id)
  : common::sig::SIGConnect(M3UA_State_NoConnection::getInstance(), "m3ua_stack",
                            remote_addr_list, remote_port,
                            local_addr_list, local_port, link_id)
{
  std::string localIfaces;
  for (std::vector<std::string>::const_iterator iter = local_addr_list.begin(), end_iter = local_addr_list.end();
      iter != end_iter; ++iter) {
    localIfaces += *iter + ",";
  }
  char portStr[32];
  sprintf(portStr, "%u", local_port);
  std::string::size_type idx = localIfaces.rfind(',');
  localIfaces.insert(idx, portStr);

  std::string remoteIfaces;
  for (std::vector<std::string>::const_iterator iter = remote_addr_list.begin(), end_iter = remote_addr_list.end();
      iter != end_iter; ++iter) {
    remoteIfaces += *iter + ",";
  }
  sprintf(portStr, "%u", remote_port);
  idx = remoteIfaces.rfind(',');
  remoteIfaces.insert(idx, portStr);

  smsc_log_info(_logger, "M3uaConnect::M3uaConnect::: connection %s==>%s, link_id=%s",
                localIfaces.c_str(), remoteIfaces.c_str(), link_id.getValue().c_str());
}

common::TP*
M3uaConnect::receive()
{
  common::TP* tp = SIGConnect::receive();
  tp->protocolClass = PROTOCOL_M3UA;
  return tp;
}

void
M3uaConnect::send(const common::Message& message)
{
  const common::AdaptationLayer_Message& suaMessage = static_cast<const common::AdaptationLayer_Message&>(message);
//  unsigned int msgClass = suaMessage.getMessageClass();
//  if ( (msgClass == common::AdaptationLayer_Message::SUA_MANAGEMENT_MESSAGES /* ERROR or NOTIFY */||
//        msgClass == common::AdaptationLayer_Message::SUA_SIGNALING_NETWORK_MANAGEMENT_MESSAGES ||
//        msgClass == common::AdaptationLayer_Message::CONNECTIONLESS_MESSAGES ) &&
//       suaMessage.isSetRoutingContext() ) {
//    suaMessage.updateRoutingContext(sua_stack::RCRegistry::getInstance().getRoutingContext(getLinkId()));
//  }
  Link::send(suaMessage);
}

void
M3uaConnect::up()
{
  messages::UPMessage upMessage;
  send(upMessage);
  smsc_log_info(_logger, "M3uaConnect::up::: send UP message to link with id=%s", getLinkId().getValue().c_str());
}

void
M3uaConnect::down()
{
  messages::DownMessage downMessage;
  send(downMessage);
  smsc_log_info(_logger, "M3uaConnect::down::: send DOWN message to link with id=%s", getLinkId().getValue().c_str());
}

void
M3uaConnect::activate()
{
  messages::ActiveMessage activeMessage;
  send(activeMessage);
  smsc_log_info(_logger, "M3uaConnect::activate::: send Active message to link with id=%s", getLinkId().getValue().c_str());
}

void
M3uaConnect::deactivate()
{
  messages::InactiveMessage inactiveMessage;
  send(inactiveMessage);
  smsc_log_info(_logger, "M3uaConnect::deactivate::: send Inactive message to link with id=%s", getLinkId().getValue().c_str());
  try {
    point_status_mgmt::PointStatusTable::getInstance().updateStatus(AdjacentDPCRegistry::getInstance().lookup(getLinkId()),
                                                                    point_status_mgmt::POINT_PROHIBITED);
  } catch (utilx::RegistryKeyNotFound) {}
}

}}}}}
