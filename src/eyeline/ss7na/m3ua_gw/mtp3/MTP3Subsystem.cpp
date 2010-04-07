#include "MTP3Subsystem.hpp"

#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/SubsystemsManager.hpp"

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

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.asp_traffic_mode", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.local_address", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.local_port", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.remote_address", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.remote_port", this);

  try {
    m3ua_stack::AspMaintenanceMessageHandlers::setSGPTrafficMode(rconfig.find<utilx::runtime_cfg::Parameter>("config.asp_traffic_mode").getValue());
  } catch (std::exception& ex) {}

  configurePoints(rconfig);
  fillInRoutingTable(rconfig);
  activateLinksToSGP(rconfig);
}

void
MTP3Subsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    if ( modified_parameter.getFullName() == "asp_traffic_mode" ) {
      smsc_log_debug(_logger, "MTP3Subsystem::handle::: handle modified 'config.asp_traffic_mode' parameter=[%s]", modified_parameter.getValue().c_str());
      utilx::runtime_cfg::Parameter* trafficModeParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("asp_traffic_mode");
      if ( trafficModeParam )
        trafficModeParam->setValue(modified_parameter.getValue());
    }
  }
}

void
MTP3Subsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                             utilx::runtime_cfg::Parameter* added_parameter)
{
  if ( context.getFullName() == "config.sgp_links.link" ) {
    utilx::runtime_cfg::CompositeParameter* linkParam = findContextParentParameter(runtime_cfg::RuntimeConfig::getInstance(),
                                                                                   context);

    if ( !linkParam ) return;

    if ( added_parameter->getName() != "local_address" &&
         added_parameter->getName() != "local_port" &&
         added_parameter->getName() != "remote_address" &&
         added_parameter->getName() != "remote_port" ) return;

    smsc_log_debug(_logger, "MTP3Subsystem::handle::: handle added parameter '%s'='%s' for Link=[%s]", added_parameter->getName().c_str(), added_parameter->getValue().c_str(), context.getValue().c_str());

    if ( !checkParameterExist(linkParam, added_parameter) )
      linkParam->addParameter(added_parameter);
  }
}

utilx::runtime_cfg::CompositeParameter*
MTP3Subsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                             utilx::runtime_cfg::CompositeParameter* added_parameter)
{
  if ( context.getFullName() == "config.sgp_links" ) {
    // this condition is true when called LM_SGPLinks_AddLinkCommand::executeCommand()
    utilx::runtime_cfg::CompositeParameter& sgpLinksParameter = 
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

    if ( added_parameter->getName() != "link" )
      generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MTP3Subsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());

    if ( checkParameterExist(&sgpLinksParameter, added_parameter) )
      generateExceptionAndForcePopUpCurrentInterpreter("Inconsistent config modification request - link with such value already exists", "MTP3Subsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());

    sgpLinksParameter.addParameter(added_parameter);
    return added_parameter;
  } else
    generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "MTP3Subsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());
  return NULL; // to exclude compiler warning "The last statement should return a value."
}

void
MTP3Subsystem::waitForCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  while(_establishedLinks > 0)
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
    common::point_code_t lpc = static_cast<common::point_code_t>(atoi(lpcParam->getValue().c_str()));

    utilx::runtime_cfg::Parameter* niParam =
        nextParameter->getParameter<utilx::runtime_cfg::Parameter>("ni");
    uint8_t ni = static_cast<uint8_t>(atoi(niParam->getValue().c_str()));

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
    common::point_code_t lpc = static_cast<common::point_code_t>(utilx::strtol(nextParameter->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getValue().c_str(), NULL, 10));
    if ( lpc == 0 && errno )
      throw smsc::util::Exception("MTP3Subsystem::fillUpRoutingTable::: invalid value of config.mtp3-routing-tables.table.lpc parameter=[%s]",
                                  nextParameter->getParameter<utilx::runtime_cfg::Parameter>("lpc")->getValue().c_str());

    msu_processor::RoutingTable* routingTable = new msu_processor::RoutingTable();

    utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> entryIterator =
        nextParameter->getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
    while(entryIterator.hasElement()) {
      const utilx::runtime_cfg::CompositeParameter* entryParameter = entryIterator.getCurrentElement();
      const std::string& entryName = entryParameter->getValue();
      const utilx::runtime_cfg::Parameter* dpcParam = entryParameter->getParameter<utilx::runtime_cfg::Parameter>("dpc");
      const utilx::runtime_cfg::Parameter* sgpLinkParam = entryParameter->getParameter<utilx::runtime_cfg::Parameter>("sgp_link");
      common::point_code_t dpc = static_cast<common::point_code_t>(utilx::strtol(dpcParam->getValue().c_str(), NULL, 10));
      if ( dpc == 0 && errno )
        throw smsc::util::Exception("MTP3Subsystem::fillUpRoutingTable::: invalid value of config.mtp3-routing-tables.table.entry.dpc parameter=[%s]",
                                    dpcParam->getValue().c_str());

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
MTP3Subsystem::activateLinksToSGP(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  utilx::runtime_cfg::CompositeParameter& sgpLinksParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
  linkIterator = sgpLinksParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("link");
  while (linkIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = linkIterator.getCurrentElement();

    common::LinkId linkIdToSgp(nextParameter->getValue());

    std::vector<std::string> l_addr, r_addr;
    in_port_t r_port = atoi(nextParameter->getParameter<utilx::runtime_cfg::Parameter>("remote_port")->getValue().c_str());
    utilx::runtime_cfg::Parameter* localPortCfgParam = nextParameter->getParameter<utilx::runtime_cfg::Parameter>("local_port");
    in_port_t l_port = 0;
    if ( localPortCfgParam )
      l_port = atoi(localPortCfgParam->getValue().c_str());

    extractAddressParameters(&l_addr,
                             nextParameter->getParameter<utilx::runtime_cfg::CompositeParameter>("local_addresses"),
                             "address");
    extractAddressParameters(&r_addr,
                             nextParameter->getParameter<utilx::runtime_cfg::CompositeParameter>("remote_addresses"),
                             "address");

    smsc_log_info(_logger, "MTP3Subsystem::initialize::: create new M3uaConnect, linkId=[%s]", linkIdToSgp.getValue().c_str());

    m3ua_stack::M3uaConnect* m3uaConnect;
    if ( l_port )
      m3uaConnect = new m3ua_stack::M3uaConnect(r_addr, r_port, l_addr, l_port, linkIdToSgp);
    else
      m3uaConnect = new m3ua_stack::M3uaConnect(r_addr, r_port, linkIdToSgp);

    m3uaConnect->setListener(this);

    smsc_log_info(_logger, "MTP3Subsystem::initialize::: try establish sctp association");
    m3uaConnect->sctpEstablish();
    ++_establishedLinks;
    SGPLinkIdsRegistry::getInstance().insert(linkIdToSgp);
    smsc_log_info(_logger, "MTP3Subsystem::initialize::: add M3uaConnect to ConnectMgr");
    io_dispatcher::ConnectMgr::getInstance().addLink(m3uaConnect->getLinkId(), m3uaConnect);

    m3uaConnect->up(); // activation for m3uaConnect will be made in AspMaintenanceMessageHandlers::handle(const UPAckMessage& message, io_dispatcher::Link* connect)

    _sgpLinkIds.push_back(m3uaConnect->getLinkId());

    linkIterator.next();
  }
}

}}}}
