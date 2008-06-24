#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <util/Exception.hpp>
#include <sua/utilx/strtol.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/communication/libsua_messages/initializer.hpp>

#include "LibSuaConnectAcceptor.hpp"
#include "SuaUserCommunicationSubsystem.hpp"
#include "ProtocolStates.hpp"
#include "MessageHandlers.hpp"
#include "LinkSetInfoRegistry.hpp"
#include "initializer.hpp"

namespace sua_user_communication {

utilx::ThreadSpecificData<bool>
SuaUserCommunicationSubsystem::_threadScopeModificationFlag;

SuaUserCommunicationSubsystem::SuaUserCommunicationSubsystem()
  : sua_layer::ApplicationSubsystem("SuaUserCommunicationSubsystem", "sua_usr_cm")
{}

void
SuaUserCommunicationSubsystem::stop()
{
  smsc_log_info(_logger, "SuaUserCommunicationSubsystem::stop::: shutdown subsystem");
  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, true);
}

void
SuaUserCommunicationSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "SuaUserCommunicationSubsystem::initialize::: try initialize SuaUserCommunication subsystem");
  LinkSetInfoRegistry::init();
  MessageHandlers::init();

  LibSua_State_NoConnection::init();
  LibSua_State_Unbind::init();
  LibSua_State_Bind::init();

  libsua_messages::initialize();
  registerMessageCreators();

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_ip", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_port", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sua_applications.application", this);

  runtime_cfg::Parameter& listeningHostParameter = rconfig.find<runtime_cfg::Parameter>("config.local_ip");
  runtime_cfg::Parameter& listeningPortParameter = rconfig.find<runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SuaUserCommunicationSubsystem::initialize::: invalid config.local_port value");

  initializeConnectAcceptor(localHost, localPort);

  runtime_cfg::CompositeParameter& sua_applications_parameter = rconfig.find<runtime_cfg::CompositeParameter>("config.sua_applications");

  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> iterator = sua_applications_parameter.getIterator<runtime_cfg::Parameter>("application");
  //iterator points to first element. If there are nothing elements in result then hasElement() returns false
  while(iterator.hasElement()) {
    const runtime_cfg::Parameter* parameter = iterator.getCurrentElement();
    const std::string& appId =  parameter->getValue();
    communication::LinkId linkSetId;
    if ( !LinkSetInfoRegistry::getInstance().getLinkSet(appId, &linkSetId) )
      throw smsc::util::Exception("SuaUserCommunicationSubsystem::initialize::: unknown application id value=[%s]", appId.c_str());
    iterator.next(); // pass to next element
  }
}

void
SuaUserCommunicationSubsystem::addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                                        runtime_cfg::Parameter* addedParameter)
{
  if ( context.getFullName() == "config.sua_applications" ) {
    runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    runtime_cfg::CompositeParameter& suaApplicationsParam = runtimeConfig.find<runtime_cfg::CompositeParameter>("config.sua_applications");

    if ( addedParameter->getName() == "application" ) {
      smsc_log_debug(_logger, "SuaUserCommunicationSubsystem::addParameterEventHandler::: handle added parameter 'config.sua_applications.application'='%s'", addedParameter->getValue().c_str());

      if ( !checkParameterExist(&suaApplicationsParam, addedParameter) )
        suaApplicationsParam.addParameter(addedParameter);
    }
  }
}

void
SuaUserCommunicationSubsystem::changeParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                                           const runtime_cfg::Parameter& modifiedParameter)
{
  if ( context.getFullName() == "config" ) {
    runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<runtime_cfg::CompositeParameter>("config");

    if ( modifiedParameter.getName() == "commit" )
      applyParametersChange();
    else {
      if ( modifiedParameter.getName() != "local_ip" &&
           modifiedParameter.getName() != "local_port" )
        return;

      smsc_log_debug(_logger, "SuaUserCommunicationSubsystem::changeParameterEventHandler::: handle modified parameter 'config.%s'='%s'", modifiedParameter.getName().c_str(), modifiedParameter.getValue().c_str());
      runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<runtime_cfg::Parameter>(modifiedParameter.getName());
      if ( !foundParam )
        throw smsc::util::Exception("SuaUserCommunicationSubsystem::changeParameterEventHandler::: parameter '%s' not found in current configuration", modifiedParameter.getName().c_str());

      foundParam->setValue(modifiedParameter.getValue());
      _threadScopeModificationFlag.set(true);
    }
  }
}

void
SuaUserCommunicationSubsystem::applyParametersChange()
{
  smsc_log_debug(_logger, "SuaUserCommunicationSubsystem::applyParametersChange::: Enter it");

  if ( !_threadScopeModificationFlag.get(false) ) return;

  _threadScopeModificationFlag.set(false);

  runtime_cfg::RuntimeConfig& rconfig = runtime_cfg::RuntimeConfig::getInstance();

  runtime_cfg::Parameter& listeningHostParameter = rconfig.find<runtime_cfg::Parameter>("config.local_ip");
  runtime_cfg::Parameter& listeningPortParameter = rconfig.find<runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SuaUserCommunicationSubsystem::applyParametersChange::: invalid config.local_port value");

  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, false); // false --> don't remove current active client's connections
  initializeConnectAcceptor(localHost, localPort);
}

void
SuaUserCommunicationSubsystem::initializeConnectAcceptor(const std::string& localHost,
                                                         in_port_t localPort)
{
  snprintf(_acceptorName, sizeof(_acceptorName), "libSuaAcptr_%s.%d", localHost.c_str(), localPort);

  char acceptorDescription[256];
  snprintf(acceptorDescription, sizeof(acceptorDescription), "LibSuaConnectAcceptor for localHost=%s,localPort=%d", localHost.c_str(), localPort);

  io_dispatcher::ConnectMgr::getInstance().addConnectAcceptor(_acceptorName, new LibSuaConnectAcceptor(acceptorDescription, localHost, localPort));

  smsc_log_info(_logger, "SuaUserCommunicationSubsystem::initialize::: ConnectAcceptor with name [%s] was initialized", _acceptorName);
}

// void
// SuaUserCommunicationSubsystem::removeParameterHandler(const runtime_cfg::Parameter& removedParameter)
// {
//   if ( removedParameter.getFullName() == "config.sua_applications.application" ) {
//     smsc_log_debug(_logger, "SuaUserCommunicationSubsystem::handle::: handle removed 'config.sua_applications.application' parameter=[%s]", removedParameter.getValue().c_str());
//   }
// }

}
