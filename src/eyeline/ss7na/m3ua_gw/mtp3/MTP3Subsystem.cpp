#include "MTP3Subsystem.hpp"

#include <stdlib.h>
#include <errno.h>

#include "eyeline/utilx/SubsystemsManager.hpp"
#include "eyeline/utilx/runtime_cfg/Exception.hpp"

#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"
#include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"

#include "m3ua_stack/initializer.hpp"
#include "m3ua_stack/ProtocolStates.hpp"
#include "m3ua_stack/M3uaConnect.hpp"
#include "m3ua_stack/AspMaintenanceMessageHandlers.hpp"
#include "m3ua_stack/DATAMessageHandler.hpp"
#include "m3ua_stack/M3UAManagementMessageHandlers.hpp"
#include "m3ua_stack/SignalingNetworkManagementMessageHandlers.hpp"
#include "m3ua_stack/messages/DownMessage.hpp"
#include "m3ua_stack/messages/InactiveMessage.hpp"
#include "m3ua_stack/messages/initializer.hpp"

#include "AdjacentDPCRegistry.hpp"
#include "PointsDefinitionRegistry.hpp"
#include "SGPLinkIdsRegistry.hpp"
#include "sap/MTPSapInstance.hpp"
#include "msu_processor/Router.hpp"
#include "msu_processor/RoutingTable.hpp"
#include "msu_processor/Descriminator.hpp"
#include "msu_processor/Distributor.hpp"

#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/point_status_mgmt/PointStatusTable.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MTP3IndicationsProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

MTP3Subsystem::MTP3Subsystem()
  : common::ApplicationSubsystem("MTP3Subsystem", "m3ua_stack"),
    _establishedLinks(0)
{}

void
MTP3Subsystem::stop()
{
  smsc_log_info(_logger, "MTP3Subsystem::stop::: shutdown subsystem");
  for (std::vector<common::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    const common::LinkId& linkId = *iter;
    smsc_log_info(_logger, "MTP3Subsystem::stop::: shutdown link [linkId=%s]", linkId.getValue().c_str());
    try {
      io_dispatcher::ConnectMgr::getInstance().send(*iter, m3ua_stack::messages::InactiveMessage());
    } catch (common::io_dispatcher::ProtocolException& ex) {
      smsc_log_info(_logger, "MTP3Subsystem::stop::: link is not in ACTIVE state, send Down message");
      try {
        io_dispatcher::ConnectMgr::getInstance().send(linkId, m3ua_stack::messages::DownMessage());
      } catch (common::io_dispatcher::ProtocolException& ex) {
        smsc_log_info(_logger, "MTP3Subsystem::stop::: link is not in INACTIVE state, remove link");
        common::io_dispatcher::LinkPtr linkPtr = io_dispatcher::ConnectMgr::getInstance().removeLink(linkId, false);
      }
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "MTP3Subsystem::stop::: caught exception [%s]", ex.what());
    }
  }
}

void
MTP3Subsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "MTP3Subsystem::initialize::: try initialize M3uaStack subsystem");
  m3ua_stack::M3UA_State_NoConnection::init();
  m3ua_stack::M3UA_State_ASPInactive::init();
  m3ua_stack::M3UA_State_ASPActive::init();
  m3ua_stack::M3UA_State_ASPDown::init();
  m3ua_stack::M3UA_State_ASPDownPending::init();
  m3ua_stack::M3UA_State_ASPInactivePending::init();
  m3ua_stack::M3UA_State_ASPActivePending::init();
  m3ua_stack::M3UA_State_ASPActiveShutdown::init();

  sap::MTPSapInstance::init();

  m3ua_stack::messages::initialize();
  m3ua_stack::registerMessageCreators();
  smsc_log_debug(_logger, "MTP3Subsystem::initialize::: m3ua_stack message creators initialized");
  point_status_mgmt::PointStatusTable::init();
  AdjacentDPCRegistry::init();
  SGPLinkIdsRegistry::init();
  PointsDefinitionRegistry::init();

  msu_processor::Descriminator::init();
  msu_processor::Distributor::init();
  msu_processor::Router::init();

  try {
    m3ua_stack::AspMaintenanceMessageHandlers::setSGPTrafficMode(rconfig.find<utilx::runtime_cfg::Parameter>("config.asp_traffic_mode").getValue());
  } catch (std::exception& ex) {}

  configurePoints(rconfig);
  fillInRoutingTable(rconfig);
  activateLinksToSGP(rconfig);
}

void
MTP3Subsystem::waitForCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  while (_establishedLinks > 0)
    _allLinksShutdownMonitor.wait();

  for (std::vector<common::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    io_dispatcher::ConnectMgr::getInstance().removeLink(*iter /*linkId*/, false);
  }

  common::io_dispatcher::ConnectMgr& connMgr = io_dispatcher::ConnectMgr::getInstance();
  utilx::SubsystemsManager::getInstance()->subscribeToShutdownInProgressEvent(&connMgr);
}

void
MTP3Subsystem::notifyLinkShutdownCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  if ( !_establishedLinks )
    return;

  --_establishedLinks;
  if ( !_establishedLinks ) _allLinksShutdownMonitor.notify();
}

void
MTP3Subsystem::extractAddressParameters(std::vector<std::string>* addrs,
                                        utilx::runtime_cfg::CompositeParameter* next_parameter,
                                        const std::string& param_name)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> addrIterator = next_parameter->getIterator<utilx::runtime_cfg::Parameter>(param_name);
  while (addrIterator.hasElement()) {
    const utilx::runtime_cfg::Parameter* nextAddrParameter = addrIterator.getCurrentElement();

    addrs->push_back(nextAddrParameter->getValue());
    addrIterator.next();
  }
}

void
MTP3Subsystem::configurePoints(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  utilx::runtime_cfg::CompositeParameter& pointsDefinitionParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.points-definition");
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    pointIterator = pointsDefinitionParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("point");
  while (pointIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = pointIterator.getCurrentElement();

    utilx::runtime_cfg::Parameter* lpcParam =
        nextParameter->getParameter<utilx::runtime_cfg::Parameter>("lpc");
    common::point_code_t lpc = lpcParam->getIntValue();

    utilx::runtime_cfg::Parameter* niParam =
        nextParameter->getParameter<utilx::runtime_cfg::Parameter>("ni");
    uint8_t ni = static_cast<uint8_t>(niParam->getIntValue());

    utilx::runtime_cfg::Parameter* standardParam =
        nextParameter->getParameter<utilx::runtime_cfg::Parameter>("standard");
    const std::string& standard = standardParam->getValue();

    PointsDefinitionRegistry::getInstance().insert(PointInfo(ni, standard), lpc);
    pointIterator.next();
  }
}

void
MTP3Subsystem::fillInRoutingTable(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  utilx::runtime_cfg::CompositeParameter& mtp3RoutingTableParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.mtp3-routing-tables");
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    tableIterator = mtp3RoutingTableParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("table");
  while (tableIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = tableIterator.getCurrentElement();

    const std::string& routingTableId = nextParameter->getValue();
    common::point_code_t lpc = static_cast<common::point_code_t>(nextParameter->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getIntValue());

    msu_processor::RoutingTable* routingTable = new msu_processor::RoutingTable();

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> entryIterator =
        nextParameter->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while(entryIterator.hasElement()) {
      const utilx::runtime_cfg::CompositeParameter* entryParameter = entryIterator.getCurrentElement();
      const std::string& entryName = entryParameter->getValue();
      const utilx::runtime_cfg::Parameter* dpcParam = entryParameter->getParameter<utilx::runtime_cfg::Parameter>("dpc");
      const utilx::runtime_cfg::Parameter* sgpLinkParam = entryParameter->getParameter<utilx::runtime_cfg::Parameter>("sgp_link");
      common::point_code_t dpc = static_cast<common::point_code_t>(dpcParam->getIntValue());
      common::LinkId sgpLinkId(sgpLinkParam->getValue());
      routingTable->addRoute(dpc, sgpLinkId);
      AdjacentDPCRegistry::getInstance().insert(dpc, sgpLinkId);
      entryIterator.next();
    }

    msu_processor::Router::getInstance().addRoutingTable(lpc, routingTable);

    tableIterator.next();
  }
}

void
MTP3Subsystem::addMtp3Route(common::point_code_t lpc, common::point_code_t dpc,
                            const common::LinkId& sgp_link_id)
{
  msu_processor::RoutingTableRefPtr routingTable =
      msu_processor::Router::getInstance().getRoutingTable(lpc);
  if ( !routingTable.Get() )
    throw smsc::util::Exception("MTP3Subsystem::addMtp3Route::: routing table not found for lpc=%u", lpc);
  routingTable->addRoute(dpc, sgp_link_id);
}

void
MTP3Subsystem::activateLinksToSGP(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  utilx::runtime_cfg::CompositeParameter& sgpLinksParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
  linkIterator = sgpLinksParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("link");
  while (linkIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = linkIterator.getCurrentElement();

    std::vector<std::string> l_addr, r_addr;
    in_port_t r_port = static_cast<in_port_t>(nextParameter->getParameter<utilx::runtime_cfg::Parameter>("remote_port")->getIntValue());
    utilx::runtime_cfg::Parameter* localPortCfgParam = nextParameter->getParameter<utilx::runtime_cfg::Parameter>("local_port");
    in_port_t l_port = 0;
    if ( localPortCfgParam )
      l_port = localPortCfgParam->getIntValue();

    extractAddressParameters(&l_addr,
                             nextParameter->getParameter<utilx::runtime_cfg::CompositeParameter>("local_addresses"),
                             "address");
    extractAddressParameters(&r_addr,
                             nextParameter->getParameter<utilx::runtime_cfg::CompositeParameter>("remote_addresses"),
                             "address");

    activateLinkToSGP(common::LinkId(nextParameter->getValue()),
                      r_addr, r_port, l_addr, l_port);

    linkIterator.next();
  }
}

void
MTP3Subsystem::activateLinkToSGP(const common::LinkId& link_id,
                                 const std::vector<std::string>& r_addr,
                                 in_port_t r_port,
                                 const std::vector<std::string>& l_addr,
                                 in_port_t l_port)
{
  smsc_log_info(_logger, "MTP3Subsystem::activateLinkToSGP::: create new M3uaConnect, linkId=[%s]", link_id.getValue().c_str());

  m3ua_stack::M3uaConnect* m3uaConnect;

  if ( l_port )
    m3uaConnect = new m3ua_stack::M3uaConnect(r_addr, r_port, l_addr, l_port, link_id);
  else
    m3uaConnect = new m3ua_stack::M3uaConnect(r_addr, r_port, link_id);

  m3uaConnect->setListener(this);

  try {
    common::io_dispatcher::LinkPtr connectGuard(m3uaConnect);
    smsc_log_info(_logger, "MTP3Subsystem::activateLinkToSGP::: try establish sctp association");
    m3uaConnect->sctpEstablish();

    SGPLinkIdsRegistry::getInstance().insert(link_id);
    smsc_log_info(_logger, "MTP3Subsystem::activateLinkToSGP::: add M3uaConnect to ConnectMgr");
    io_dispatcher::ConnectMgr::getInstance().addLink(m3uaConnect->getLinkId(), connectGuard);

    m3uaConnect->up(); // activation for m3uaConnect will be made in AspMaintenanceMessageHandlers::handle(const UPAckMessage& message, io_dispatcher::Link* connect)
    smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
    ++_establishedLinks;
    _sgpLinkIds.push_back(m3uaConnect->getLinkId());
  } catch (...) {
    io_dispatcher::ConnectMgr::getInstance().removeLink(m3uaConnect->getLinkId());
  }
}

}}}}
