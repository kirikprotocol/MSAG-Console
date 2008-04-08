#include "IODispatcherSubsystem.hpp"
#include "ConnectMgr.hpp"
#include "IOEventProcessor.hpp"
#include "SuaLayerMessagesFactory.hpp"

namespace io_dispatcher {

IODispatcherSubsystem*
utilx::Singleton<IODispatcherSubsystem*>::_instance;


IODispatcherSubsystem::IODispatcherSubsystem()
  : _name("IODispatcherSubsystem"), _logger(smsc::logger::Logger::getInstance("io_dsptch")),
    _stateMachinesCount(0)
{}

void
IODispatcherSubsystem::start() {
  _threadsPool.preCreateThreads(_stateMachinesCount);
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: Starting IOEventProcessors" );

  for(int i=0;i<_stateMachinesCount;i++)
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
IODispatcherSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
{
  SuaLayerMessagesFactory::init();
  //  LinkSetsManager::init();
  ConnectMgr::init();

  runtime_cfg::Parameter& stateMachinesCount = rconfig.find<runtime_cfg::Parameter>("config.state_machines_count");
  _stateMachinesCount = atoi(stateMachinesCount.getValue().c_str());
  if ( _stateMachinesCount <= 0 )
    throw smsc::util::Exception("IODispatcherSubsystem::initialize::: invalid value of config.state_machines_count parameter = %d", _stateMachinesCount);
}

const std::string&
IODispatcherSubsystem::getName() const
{
  return _name;
}

void
IODispatcherSubsystem::waitForCompletion()
{}

}
