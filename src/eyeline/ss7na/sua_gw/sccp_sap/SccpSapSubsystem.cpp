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

utilx::ThreadSpecificData<bool>
SccpSapSubsystem::_threadScopeModificationFlag;

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

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_ip", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_port", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications.application", this);

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
SccpSapSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                        utilx::runtime_cfg::Parameter* added_parameter)
{
  if ( context.getFullName() == "config.sua_applications" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& suaApplicationsParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sua_applications");

    if ( added_parameter->getName() == "application" ) {
      smsc_log_debug(_logger, "SccpSapSubsystem::addParameterEventHandler::: handle added parameter 'config.sua_applications.application'='%s'", added_parameter->getValue().c_str());

      if ( !checkParameterExist(&suaApplicationsParam, added_parameter) )
        suaApplicationsParam.addParameter(added_parameter);
    }
  }
}

void
SccpSapSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                           const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    if ( modified_parameter.getName() == "commit" )
      applyParametersChange();
    else {
      if ( modified_parameter.getName() != "local_ip" &&
           modified_parameter.getName() != "local_port" )
        return;

      smsc_log_debug(_logger, "SccpSapSubsystem::changeParameterEventHandler::: handle modified parameter 'config.%s'='%s'", modified_parameter.getName().c_str(), modified_parameter.getValue().c_str());
      utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>(modified_parameter.getName());
      if ( !foundParam )
        throw smsc::util::Exception("SccpSapSubsystem::changeParameterEventHandler::: parameter '%s' not found in current configuration", modified_parameter.getName().c_str());

      foundParam->setValue(modified_parameter.getValue());
      _threadScopeModificationFlag.set(true);
    }
  }
}

void
SccpSapSubsystem::applyParametersChange()
{
  smsc_log_debug(_logger, "SccpSapSubsystem::applyParametersChange::: Enter it");

  if ( !_threadScopeModificationFlag.get(false) ) return;

  _threadScopeModificationFlag.set(false);

  utilx::runtime_cfg::RuntimeConfig& rconfig = runtime_cfg::RuntimeConfig::getInstance();

  utilx::runtime_cfg::Parameter& listeningHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_ip");
  utilx::runtime_cfg::Parameter& listeningPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SccpSapSubsystem::applyParametersChange::: invalid config.local_port value");

  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, false); // false --> don't remove current active client's connections
  initializeConnectAcceptor(localHost, localPort);
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
