#include <set>

#include "AspMaintenanceMessageHandlers.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/AdjacentDPCRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/point_status_mgmt/PointStatusTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

AspMaintenanceMessageHandlers::AspMaintenanceMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("m3ua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance())
{}

void
AspMaintenanceMessageHandlers::handle(const messages::ActiveMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_error(_logger, "AspMaintenanceMessageHandlers::handle::: handle ActiveMessage [%s]", message.toString().c_str());
  throw common::io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: ActiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const messages::ActiveAckMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle ActiveAckMessage [%s]", message.toString().c_str());
  point_status_mgmt::PointStatusTable::getInstance().updateStatus(AdjacentDPCRegistry::getInstance().lookup(link_id),
                                                                  point_status_mgmt::POINT_ALLOWED);
}

void
AspMaintenanceMessageHandlers::handle(const messages::InactiveMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle InactiveMessage [%s]", message.toString().c_str());
  throw common::io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: InactiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const messages::InactiveAckMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle InactiveAckMessage [%s]", message.toString().c_str());
  messages::DownMessage downMessage;
  _cMgr.send(link_id, downMessage);
}

void
AspMaintenanceMessageHandlers::handle(const messages::UPMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle UPMessage [%s]", message.toString().c_str());
  throw common::io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: ActiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const messages::UPAckMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle UPAckMessage [%s]", message.toString().c_str());

  messages::ActiveMessage activeMessage;
  if ( _trafficMode != common::io_dispatcher::LinkSet::UNSPECIFIED )
    activeMessage.setTrafficModyType(common::TLV_TrafficModeType(_trafficMode));

  _cMgr.send(link_id, activeMessage);
}

void
AspMaintenanceMessageHandlers::handle(const messages::DownMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle DownMessage [%s]", message.toString().c_str());
  throw common::io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: DownMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const messages::DownAckMessage& message,
                                      const common::LinkId& link_id)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle DownAckMessage [%s]", message.toString().c_str());
  common::io_dispatcher::LinkPtr linkPtr = _cMgr.removeLink(link_id, false);
  // connection closing will make in desctructor
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: link=[%s] has been removed from ConnectMgr", linkPtr->getLinkId().getValue().c_str());
}

void
AspMaintenanceMessageHandlers::setSGPTrafficMode(const std::string& traffic_mode_value)
{
  if ( !strcasecmp(traffic_mode_value.c_str(), "loadshare") )
    _trafficMode = common::io_dispatcher::LinkSet::LOADSHARE;
  else if ( !strcasecmp(traffic_mode_value.c_str(), "override") )
    _trafficMode = common::io_dispatcher::LinkSet::OVERRIDE;
  else if ( !strcasecmp(traffic_mode_value.c_str(), "broadcast") )
    _trafficMode = common::io_dispatcher::LinkSet::BROADCAST;
  else
    throw smsc::util::Exception("AspMaintenanceMessageHandlers::setSGPTrafficMode::: wrong traffic-mode parameter value=[%s]", traffic_mode_value.c_str());
}

uint32_t AspMaintenanceMessageHandlers::_trafficMode;

}}}}}
