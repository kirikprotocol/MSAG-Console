#include <eyeline/utilx/strtol.hpp>
#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

#include "IODispatcherSubsystem.hpp"
#include "ConnectMgr.hpp"
#include "IOEventProcessor.hpp"
#include "SuaLayerMessagesFactory.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

IODispatcherSubsystem::IODispatcherSubsystem()
  : sua_layer::ApplicationSubsystem("IODispatcherSubsystem", "io_dsptch"),
    _stateMachinesCount(0)
{}

void
IODispatcherSubsystem::start() {
  _threadsPool.preCreateThreads(static_cast<int>(_stateMachinesCount));
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: Starting IOEventProcessors" );

  for(size_t i=0;i<_stateMachinesCount;i++)
  {
    IOEventProcessor *ioEventProcessor=new IOEventProcessor(ConnectMgr::getInstance());
    smsc_log_debug(_logger, "IODispatcherSubsystem::start::: start IOEventProcessor");
    _threadsPool.startTask(ioEventProcessor);
  }
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: IOEventProcessors have started" );
}

void
IODispatcherSubsystem::stop() {
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: stoping IOEventProcessorsPool" );
  _threadsPool.shutdown();
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: IOEventProcessorsPool stopped" );
}

void
IODispatcherSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  SuaLayerMessagesFactory::init();
  ConnectMgr::init();

  utilx::runtime_cfg::Parameter& stateMachinesCount = rconfig.find<utilx::runtime_cfg::Parameter>("config.state_machines_count");
  _stateMachinesCount = utilx::strtol(stateMachinesCount.getValue().c_str(), (char **)NULL, 10);
  if ( _stateMachinesCount == 0 && errno )
    throw smsc::util::Exception("IODispatcherSubsystem::initialize::: invalid value of config.state_machines_count parameter=[%s]", stateMachinesCount.getValue().c_str());
}

void
IODispatcherSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                                   const utilx::runtime_cfg::Parameter& modifiedParameter)
{
  if ( context.getFullName() == "config" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    if ( modifiedParameter.getFullName() != "state_machines_count" ) return;

    smsc_log_debug(_logger, "IODispatcherSubsystem::handle::: handle modified parameter 'config.%s'='%s'", modifiedParameter.getName().c_str(), modifiedParameter.getValue().c_str());

    utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>(modifiedParameter.getName());
    if ( foundParam )
      foundParam->setValue(modifiedParameter.getValue());
  }
}

}}}}
