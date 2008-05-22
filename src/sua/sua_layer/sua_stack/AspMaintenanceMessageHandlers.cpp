#include <set>
#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/communication/sua_messages/SuaTLV.hpp>
#include "AspMaintenanceMessageHandlers.hpp"
#include "SuaConnect.hpp"
#include "LinkSetsRegistry.hpp"
#include "RCRegistry.hpp"

sua_stack::AspMaintenanceMessageHandlers*
utilx::Singleton<sua_stack::AspMaintenanceMessageHandlers>::_instance;

namespace sua_stack {

AspMaintenanceMessageHandlers::AspMaintenanceMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sua_stack")), _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _trafficMode(io_dispatcher::LinkSet::UNSPECIFIED) {}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::ActiveMessage& message, const communication::LinkId& linkId)
{
  smsc_log_error(_logger, "AspMaintenanceMessageHandlers::handle::: handle ActiveMessage [%s]", message.toString().c_str());
  throw io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: ActiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::ActiveAckMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle ActiveAckMessage [%s]", message.toString().c_str());

  std::set<communication::LinkId> linkSetIds = LinkSetsRegistry::getInstance().getLinkSetsByLink(linkId);
  for(std::set<communication::LinkId>::iterator iter = linkSetIds.begin(), end_iter = linkSetIds.end(); iter != end_iter; ++iter) {
    _cMgr.addLinkToLinkSet(*iter, linkId);
  }
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::InactiveMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle InactiveMessage [%s]", message.toString().c_str());
  throw io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: InactiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::InactiveAckMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle InactiveAckMessage [%s]", message.toString().c_str());
  _cMgr.removeLinkFromLinkSets(_cMgr.getLinkSetIds(linkId), linkId);
  RCRegistry::getInstance().remove(linkId);
  sua_messages::DownMessage downMessage;
  _cMgr.send(linkId, downMessage);
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::UPMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle UPMessage [%s]", message.toString().c_str());
  throw io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: ActiveMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::UPAckMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle UPAckMessage [%s]", message.toString().c_str());

  sua_messages::ActiveMessage activeMessage;
  if ( _trafficMode != io_dispatcher::LinkSet::UNSPECIFIED )
    activeMessage.setTrafficModyType(sua_messages::TLV_TrafficModeType(_trafficMode));

  _cMgr.send(linkId, activeMessage);
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::DownMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle DownMessage [%s]", message.toString().c_str());
  throw io_dispatcher::ProtocolException("AspMaintenanceMessageHandlers::handle::: DownMessage isn't accepted - IPSP-IPSP configuration not supported.");
}

void
AspMaintenanceMessageHandlers::handle(const sua_messages::DownAckMessage& message, const communication::LinkId& linkId)
{
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: handle DownAckMessage [%s]", message.toString().c_str());
  io_dispatcher::LinkPtr linkPtr = _cMgr.removeLink(linkId, false);
  // connection closing will make in desctructor
  smsc_log_info(_logger, "AspMaintenanceMessageHandlers::handle::: link=[%s] has been removed from ConnectMgr", linkPtr->getLinkId().getValue().c_str());
}


void
AspMaintenanceMessageHandlers::setSGPTrafficMode(const std::string& trafficModeValue)
{
  if ( !strcasecmp(trafficModeValue.c_str(), "loadshare") )
    _trafficMode = io_dispatcher::LinkSet::LOADSHARE;
  else if ( !strcasecmp(trafficModeValue.c_str(), "override") )
    _trafficMode = io_dispatcher::LinkSet::OVERRIDE;
  else if ( !strcasecmp(trafficModeValue.c_str(), "broadcast") )
    _trafficMode = io_dispatcher::LinkSet::BROADCAST;
  else
    throw smsc::util::Exception("AspMaintenanceMessageHandlers::setSGPTrafficMode::: wrong traffic-mode parameter value=[%s]", trafficModeValue.c_str());
}

}
