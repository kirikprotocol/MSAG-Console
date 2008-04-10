#include <stdio.h>
#include <stdlib.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <sua/communication/libsua_messages/initializer.hpp>

#include "LibSuaConnectAcceptor.hpp"
#include "SuaUserCommunicationSubsystem.hpp"
#include "ProtocolStates.hpp"
#include "MessageHandlers.hpp"
#include "LinkSetInfoRegistry.hpp"
#include "initializer.hpp"

namespace sua_user_communication {

SuaUserCommunicationSubsystem*
utilx::Singleton<SuaUserCommunicationSubsystem*>::_instance;

SuaUserCommunicationSubsystem::SuaUserCommunicationSubsystem()
  : _name("SuaUserCommunicationSubsystem"), _logger(smsc::logger::Logger::getInstance("sua_usr_cm"))
{}

void
SuaUserCommunicationSubsystem::start()
{}

void
SuaUserCommunicationSubsystem::stop()
{
  smsc_log_info(_logger, "SuaUserCommunicationSubsystem::stop::: shutdown subsystem");
  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName);
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

  runtime_cfg::Parameter& listeningHostParameter = rconfig.find<runtime_cfg::Parameter>("config.local_ip");
  runtime_cfg::Parameter& listeningPortParameter = rconfig.find<runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)atoi(listeningPortParameter.getValue().c_str());
  snprintf(_acceptorName, sizeof(_acceptorName), "libSuaAcptr_%s.%d", localHost.c_str(), localPort);

  char acceptorDescription[256];
  snprintf(acceptorDescription, sizeof(acceptorDescription), "LibSuaConnectAcceptor for localHost=%s,localPort=%d", localHost.c_str(), localPort);

  io_dispatcher::ConnectMgr::getInstance().addConnectAcceptor(_acceptorName, new LibSuaConnectAcceptor(acceptorDescription, localHost, localPort));

  smsc_log_info(_logger, "SuaUserCommunicationSubsystem::initialize::: added ConnectAcceptor with name [%s]", _acceptorName);
  runtime_cfg::CompositeParameter& sua_applications_parameter = rconfig.find<runtime_cfg::CompositeParameter>("config.sua_applications");

  runtime_cfg::CompositeParameter::Iterator<runtime_cfg::Parameter> iterator = sua_applications_parameter.getIterator<runtime_cfg::Parameter>("application");
  //iterator points to first element. If there are nothing elements in result then hasElement() returns false
  while(iterator.hasElement()) {
    const runtime_cfg::Parameter* parameter = iterator.getCurrentElement();
    const std::string& appId =  parameter->getValue();
    LinkSetInfoRegistry::getInstance().getLinkSet(appId); // throws exception if there isn't registered appId
    iterator.next(); // pass to next element
  }
}

const std::string&
SuaUserCommunicationSubsystem::getName() const
{
  return _name;
}

void
SuaUserCommunicationSubsystem::waitForCompletion()
{}

}
