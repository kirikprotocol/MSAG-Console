#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <util/Exception.hpp>

#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

#include "eyeline/ss7na/common/sccp_sap/ProtocolStates.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"

#include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

#include "LibSccpConnectAcceptor.hpp"
#include "LinkSetInfoRegistry.hpp"
#include "SccpSapSubsystem.hpp"
#include "MessageHandlers.hpp"
#include "initializer.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

SccpSapSubsystem::SccpSapSubsystem()
  : common::ApplicationSubsystem("SccpSapSubsystem", "sua_usr_cm")
{}

void
SccpSapSubsystem::stop()
{
  smsc_log_info(_logger, "SccpSapSubsystem::stop::: shutdown subsystem");
  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, true);
}

void
SccpSapSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "SccpSapSubsystem::initialize::: try initialize SuaUserCommunication subsystem");
  LinkSetInfoRegistry::init();

  common::sccp_sap::LibSccp_State_NoConnection::init();
  common::sccp_sap::LibSccp_State_Unbind::init();
  common::sccp_sap::LibSccp_State_Bind::init();

  common::sccp_sap::ActiveAppsRegistry::init();

  registerMessageCreators();

  utilx::runtime_cfg::Parameter& listeningHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_ip");
  utilx::runtime_cfg::Parameter& listeningPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SccpSapSubsystem::initialize::: invalid config.local_port value");

  initializeConnectAcceptor(localHost, localPort);

  utilx::runtime_cfg::CompositeParameter& sua_applications_parameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.sua_applications");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> iterator = sua_applications_parameter.getIterator<utilx::runtime_cfg::Parameter>("application");
  //iterator points to first element. If there are nothing elements in result then hasElement() returns false
  while(iterator.hasElement()) {
    const utilx::runtime_cfg::Parameter* parameter = iterator.getCurrentElement();
    const std::string& appId =  parameter->getValue();
    common::LinkId linkSetId;
    if ( !LinkSetInfoRegistry::getInstance().getLinkSet(appId, &linkSetId) )
      throw smsc::util::Exception("SccpSapSubsystem::initialize::: unknown application id value=[%s]", appId.c_str());
    iterator.next(); // pass to next element
  }
}

void
SccpSapSubsystem::initializeConnectAcceptor(const std::string& local_host,
                                            in_port_t local_port)
{
  snprintf(_acceptorName, sizeof(_acceptorName), "libSuaAcptr_%s.%d", local_host.c_str(), local_port);

  char acceptorDescription[256];
  snprintf(acceptorDescription, sizeof(acceptorDescription), "LibSuaConnectAcceptor for localHost=%s,localPort=%d", local_host.c_str(), local_port);

  io_dispatcher::ConnectMgr::getInstance().addConnectAcceptor(_acceptorName,
                                                              new LibSccpConnectAcceptor(acceptorDescription, local_host, local_port));

  smsc_log_info(_logger, "SccpSapSubsystem::initialize::: ConnectAcceptor with name [%s] was initialized", _acceptorName);
}

}}}}
