#include "SuaStackSubsystem.hpp"

#include <stdlib.h>
#include <netinet/in.h>

#include "eyeline/utilx/SubsystemsManager.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

#include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"
#include "eyeline/ss7na/common/sig/LinkSetsRegistry.hpp"

#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/sua_stack/ProtocolStates.hpp"
#include "eyeline/ss7na/sua_gw/sua_stack/SuaConnect.hpp"
#include "eyeline/ss7na/sua_gw/sua_stack/messages/InactiveMessage.hpp"
#include "eyeline/ss7na/sua_gw/sua_stack/messages/initializer.hpp"

#include "ProtocolStates.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "CLCOMessageHandlers.hpp"
#include "SUAManagementMessageHandlers.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "RCRegistry.hpp"

#include "initializer.hpp"
#include "ReassemblyProcessor.hpp"
#include "eyeline/utilx/strtol.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

SuaStackSubsystem::SuaStackSubsystem()
  : common::ApplicationSubsystem("SuaStackSubsystem", "sua_stack"), _establishedLinks(0)
{}

void
SuaStackSubsystem::stop()
{
  smsc_log_info(_logger, "SuaStackSubsystem::stop::: shutdown subsytem");
  for (std::vector<common::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    const common::LinkId& linkId = *iter;
    smsc_log_info(_logger, "SuaStackSubsystem::stop::: shutdown link [linkId=%s]", linkId.getValue().c_str());
    try {
      io_dispatcher::ConnectMgr::getInstance().send(*iter, messages::InactiveMessage());
    } catch (common::io_dispatcher::ProtocolException& ex) {
      smsc_log_info(_logger, "SuaStackSubsystem::stop::: link is not in ACTIVE state, send Down message");
      try {
        io_dispatcher::ConnectMgr::getInstance().send(linkId, messages::DownMessage());
      } catch (common::io_dispatcher::ProtocolException& ex) {
        smsc_log_info(_logger, "SuaStackSubsystem::stop::: link is not in INACTIVE state, remove link");
        common::io_dispatcher::LinkPtr linkPtr = io_dispatcher::ConnectMgr::getInstance().removeLink(linkId, false);
      }
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "SuaStackSubsystem::stop::: caught exception [%s]", ex.what());
    }
  }
}

void
SuaStackSubsystem::extractAddressParameters(std::vector<std::string>* addrs,
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
SuaStackSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "SuaStackSubsystem::initialize::: try initialize SuaStack subsystem");
  SUA_State_NoConnection::init();
  SUA_State_ASPInactive::init();
  SUA_State_ASPActive::init();
  SUA_State_ASPDown::init();
  SUA_State_ASPDownPending::init();
  SUA_State_ASPInactivePending::init();
  SUA_State_ASPActivePending::init();
  SUA_State_ASPActiveShutdown::init();

  AspMaintenanceMessageHandlers::init();
  CLCOMessageHandlers::init();
  SUAManagementMessageHandlers::init();
  SignalingNetworkManagementMessageHandlers::init();

  RCRegistry::init();

  messages::initialize();
  registerMessageCreators();
  ReassemblyProcessor::init();
  unsigned reassembleTimerValue = 1;

  try {
    const utilx::runtime_cfg::Parameter& reassemblyTimerParam =
        rconfig.find<utilx::runtime_cfg::Parameter>("config.reassembly_timer");
    reassembleTimerValue = static_cast<unsigned>(utilx::strtol(reassemblyTimerParam.getValue().c_str(), (char **)NULL, 10));

    if ( reassembleTimerValue == 0 && errno )
      throw smsc::util::Exception("SuaStackSubsystem::initialize::: invalid config.reassembly_timer value='%s'",
                                  reassemblyTimerParam.getValue().c_str());
  } catch (std::runtime_error& ex) {}

  ReassemblyProcessor::getInstance().initialize(reassembleTimerValue);

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.traffic-mode-for-sgp", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.local_address", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.local_port", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.remote_address", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sgp_links.link.remote_port", this);

  try {
    utilx::runtime_cfg::Parameter& trafficModeForSGP = rconfig.find<utilx::runtime_cfg::Parameter>("config.traffic-mode-for-sgp");
    AspMaintenanceMessageHandlers::getInstance().setSGPTrafficMode(trafficModeForSGP.getValue());
  } catch (std::exception& ex) {}

  utilx::runtime_cfg::CompositeParameter& sgpLinksParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> linkIterator = sgpLinksParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("link");
  while (linkIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = linkIterator.getCurrentElement();

    common::LinkId linkIdToSgp(nextParameter->getValue());

    std::vector<std::string> l_addr, r_addr;
    in_port_t r_port = atoi(nextParameter->getParameter<utilx::runtime_cfg::Parameter>("remote_port")->getValue().c_str());
    utilx::runtime_cfg::Parameter* localPortCfgParam = nextParameter->getParameter<utilx::runtime_cfg::Parameter>("local_port");
    in_port_t l_port = 0;
    if ( localPortCfgParam )
      l_port = atoi(localPortCfgParam->getValue().c_str());

    extractAddressParameters(&l_addr, nextParameter, "local_address");
    extractAddressParameters(&r_addr, nextParameter, "remote_address");

    smsc_log_info(_logger, "SuaStackSubsystem::initialize::: create new SuaConnect, linkId=[%s]", linkIdToSgp.getValue().c_str());
    if ( ! common::sig::LinkSetsRegistry::getInstance().checkLinkIsMemberOfAnyLinkset(linkIdToSgp) )
      throw smsc::util::Exception("SuaStackSubsystem::initialize::: wrong configuration - linkId=[%s] isn't a member of any LinkSet");

    SuaConnect* suaConnect;
    if ( l_port ) 
      suaConnect = new SuaConnect(r_addr, r_port, l_addr, l_port, linkIdToSgp);
    else
      suaConnect = new SuaConnect(r_addr, r_port, linkIdToSgp);

    suaConnect->setListener(this);

    smsc_log_info(_logger, "SuaStackSubsystem::initialize::: try establish sctp association");
    suaConnect->sctpEstablish();
    ++_establishedLinks;

    smsc_log_info(_logger, "SuaStackSubsystem::initialize::: add SuaConnect to ConnectMgr");
    io_dispatcher::ConnectMgr::getInstance().addLink(suaConnect->getLinkId(), suaConnect);

    suaConnect->up(); // activation for suaConnect will be made in AspMaintenanceMessageHandlers::handle(const UPAckMessage& message, io_dispatcher::Link* connect)

    _sgpLinkIds.push_back(suaConnect->getLinkId());

    linkIterator.next();
  }
}

void
SuaStackSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                               const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    if ( modified_parameter.getFullName() == "traffic-mode-for-sgp" ) {
      smsc_log_debug(_logger, "SuaStackSubsystem::handle::: handle modified 'config.traffic-mode-for-sgp' parameter=[%s]", modified_parameter.getValue().c_str());
      utilx::runtime_cfg::Parameter* trafficModeParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>("traffic-mode-for-sgp");
      if ( trafficModeParam )
        trafficModeParam->setValue(modified_parameter.getValue());
    }
  }
}

void
SuaStackSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
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

    smsc_log_debug(_logger, "SuaStackSubsystem::handle::: handle added parameter '%s'='%s' for Link=[%s]", added_parameter->getName().c_str(), added_parameter->getValue().c_str(), context.getValue().c_str());

    if ( !checkParameterExist(linkParam, added_parameter) )
      linkParam->addParameter(added_parameter);
  }
}

utilx::runtime_cfg::CompositeParameter*
SuaStackSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                            utilx::runtime_cfg::CompositeParameter* added_parameter)
{
  if ( context.getFullName() == "config.sgp_links" ) {
    // this condition is true when called LM_SGPLinks_AddLinkCommand::executeCommand()
    utilx::runtime_cfg::CompositeParameter& sgpLinksParameter = 
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.sgp_links");

    if ( added_parameter->getName() != "link" )
      generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "SuaStackSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());


    if ( checkParameterExist(&sgpLinksParameter, added_parameter) )
      generateExceptionAndForcePopUpCurrentInterpreter("Inconsistent config modification request - link with such value already exists", "SuaStackSubsystem::addParameterEventHandler::: can't process parameter '%s'='%s' - the parameter with such value already exist", added_parameter->getName().c_str(), added_parameter->getValue().c_str());

    sgpLinksParameter.addParameter(added_parameter);
    return added_parameter;
  } else
    generateExceptionAndForcePopUpCurrentInterpreter("Error: Invalid input", "SuaStackSubsystem::addParameterEventHandler::: invalid parameter '%s' for context '%s'", added_parameter->getName().c_str(), context.getName().c_str());
}

void
SuaStackSubsystem::waitForCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  while(_establishedLinks > 0)
    _allLinksShutdownMonitor.wait();

  for (std::vector<common::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    const common::LinkId& linkId = *iter;
    std::set<common::LinkId> linkSetIds = io_dispatcher::ConnectMgr::getInstance().getLinkSetIds(linkId);
    io_dispatcher::ConnectMgr::getInstance().removeLinkFromLinkSets(linkSetIds, linkId);
    io_dispatcher::ConnectMgr::getInstance().removeLink(linkId, false);
  }

  common::io_dispatcher::ConnectMgr& connMgr = io_dispatcher::ConnectMgr::getInstance();
  utilx::SubsystemsManager::getInstance()->subscribeToShutdownInProgressEvent(&connMgr);
}

void
SuaStackSubsystem::notifyLinkShutdownCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  --_establishedLinks;
  if ( !_establishedLinks ) _allLinksShutdownMonitor.notify();
}

}}}}
