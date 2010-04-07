#include <set>

#include "messages/SuaTLV.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "SuaConnect.hpp"
#include "RCRegistry.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/common/sig/LinkSetsRegistry.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"
#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Reconnector.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

AspMaintenanceMessageHandlers::AspMaintenanceMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _trafficMode(common::io_dispatcher::LinkSet::UNSPECIFIED) {}

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

  std::set<common::LinkId> linkSetIds = common::sig::LinkSetsRegistry::getInstance().getLinkSetsByLink(link_id);
  for(std::set<common::LinkId>::iterator iter = linkSetIds.begin(), end_iter = linkSetIds.end(); iter != end_iter; ++iter) {
    _cMgr.addLinkToLinkSet(*iter, link_id);
  }
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
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(link_id), link_id);
  RCRegistry::getInstance().remove(link_id);
  messages::DownMessage downMessage;
  try {
    _cMgr.send(link_id, downMessage);
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "AspMaintenanceMessageHandlers::handle::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), link_id.getValue().c_str());
    common::io_dispatcher::LinkPtr brokeConn = _cMgr.removeLink(link_id);
    if ( brokeConn.Get() )
      common::io_dispatcher::Reconnector::getInstance().schedule(brokeConn);
  }
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

  try {
    _cMgr.send(link_id, activeMessage);
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_error(_logger, "AspMaintenanceMessageHandlers::handle::: caught exception BrokenPipe='%s' for link ='%s'",
                   ex.what(), link_id.getValue().c_str());
    common::io_dispatcher::LinkPtr brokeConn = _cMgr.removeLink(link_id);
    if ( brokeConn.Get() )
      common::io_dispatcher::Reconnector::getInstance().schedule(brokeConn);
  }
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

}}}}
