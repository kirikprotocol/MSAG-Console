#include "SuaStackSubsystem.hpp"

#include <stdlib.h>
#include <netinet/in.h>
#include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>

#include <sua/sua_layer/sua_stack/ProtocolStates.hpp>
#include <sua/sua_layer/sua_stack/SuaConnect.hpp>

#include <sua/communication/sua_messages/InactiveMessage.hpp>
#include <sua/communication/sua_messages/initializer.hpp>

#include <sua/utilx/SubsystemsManager.hpp>

#include "ProtocolStates.hpp"
#include "AspMaintenanceMessageHandlers.hpp"
#include "CLCOMessageHandlers.hpp"
#include "SUAManagementMessageHandlers.hpp"
#include "SignalingNetworkManagementMessageHandlers.hpp"
#include "RCRegistry.hpp"
#include "LinkSetsRegistry.hpp"
#include "initializer.hpp"

namespace sua_stack {

SuaStackSubsystem::SuaStackSubsystem()
  : _name("SuaStackSubsystem"), _logger(smsc::logger::Logger::getInstance("sua_stack")),
    _establishedLinks(0)
{}

void
SuaStackSubsystem::start() {}

void
SuaStackSubsystem::stop()
{
  smsc_log_info(_logger, "SuaStackSubsystem::stop::: shutdown subsytem");
  for (std::vector<communication::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    const communication::LinkId& linkId = *iter;
    smsc_log_info(_logger, "SuaStackSubsystem::stop::: shutdown link [linkId=%s]", linkId.getValue().c_str());
    try {
      io_dispatcher::ConnectMgr::getInstance().send(*iter, sua_messages::InactiveMessage());
    } catch (io_dispatcher::ProtocolException& ex) {
      smsc_log_info(_logger, "SuaStackSubsystem::stop::: link is not in ACTIVE state, send Down message");
      try {
        io_dispatcher::ConnectMgr::getInstance().send(linkId, sua_messages::DownMessage());
      } catch (io_dispatcher::ProtocolException& ex) {
        smsc_log_info(_logger, "SuaStackSubsystem::stop::: link is not in INACTIVE state, remove link");
        io_dispatcher::LinkPtr linkPtr = io_dispatcher::ConnectMgr::getInstance().removeLink(linkId, false);
      }
    }
  }
}

void
SuaStackSubsystem::extractAddressParameters(std::vector<std::string>* addrs,
                                            const runtime_cfg::CompositeParameter* nextParameter,
                                            const std::string paramName)
{
  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> addrIterator = nextParameter->getIterator<runtime_cfg::Parameter>(paramName);
  while (addrIterator.hasElement()) {
    const runtime_cfg::Parameter* nextAddrParameter = addrIterator.getCurrentElement();
    
    addrs->push_back(nextAddrParameter->getValue());
    addrIterator.next();
  }
}


void
SuaStackSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
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

  sua_messages::initialize();
  registerMessageCreators();

  try {
    runtime_cfg::Parameter& trafficModeForSGP = rconfig.find<runtime_cfg::Parameter>("config.traffic-mode-for-sgp");
    AspMaintenanceMessageHandlers::getInstance().setSGPTrafficMode(trafficModeForSGP.getValue());
  } catch (std::exception& ex) {}

  runtime_cfg::CompositeParameter& sgpLinksParameter = rconfig.find<runtime_cfg::CompositeParameter>("config.sgp_links");

  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::CompositeParameter> linkIterator = sgpLinksParameter.getIterator<runtime_cfg::CompositeParameter>("link");
  while (linkIterator.hasElement()) {
    const runtime_cfg::CompositeParameter* nextParameter = linkIterator.getCurrentElement();

    communication::LinkId linkIdToSgp(nextParameter->getValue());

    std::vector<std::string> l_addr, r_addr;
    in_port_t r_port = atoi(nextParameter->getParameter<runtime_cfg::Parameter>("remote_port")->getValue().c_str());
    runtime_cfg::Parameter* localPortCfgParam = nextParameter->getParameter<runtime_cfg::Parameter>("local_port");
    in_port_t l_port = 0;
    if ( localPortCfgParam )
      l_port = atoi(localPortCfgParam->getValue().c_str());

    extractAddressParameters(&l_addr, nextParameter, "local_address");
    extractAddressParameters(&r_addr, nextParameter, "remote_address");

    smsc_log_info(_logger, "SuaStackSubsystem::initialize::: create new SuaConnect, linkId=[%s]", linkIdToSgp.getValue().c_str());
    if ( ! LinkSetsRegistry::getInstance().checkLinkIsMemberOfAnyLinkset(linkIdToSgp) )
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

const std::string&
SuaStackSubsystem::getName() const
{
  return _name;
}

void
SuaStackSubsystem::waitForCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  while(_establishedLinks > 0)
    _allLinksShutdownMonitor.wait();

  for (std::vector<communication::LinkId>::iterator iter = _sgpLinkIds.begin(), end_iter = _sgpLinkIds.end();
       iter != end_iter; ++iter) {
    const communication::LinkId& linkId = *iter;
    std::set<communication::LinkId> linkSetIds = io_dispatcher::ConnectMgr::getInstance().getLinkSetIds(linkId);
    io_dispatcher::ConnectMgr::getInstance().removeLinkFromLinkSets(linkSetIds, linkId);
    io_dispatcher::ConnectMgr::getInstance().removeLink(linkId, false);
  }

  io_dispatcher::ConnectMgr& connMgr = io_dispatcher::ConnectMgr::getInstance();
  utilx::SubsystemsManager::getInstance()->subscribeToShutdownInProgressEvent(&connMgr);
}

void
SuaStackSubsystem::notifyLinkShutdownCompletion()
{
  smsc::core::synchronization::MutexGuard lock(_allLinksShutdownMonitor);
  --_establishedLinks;
  if ( !_establishedLinks ) _allLinksShutdownMonitor.notify();
}

}
