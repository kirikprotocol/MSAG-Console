#include "util/Exception.hpp"
#include "eyeline/utilx/strtol.hpp"

#include "ConnectMgr.hpp"
#include "MessagesFactory.hpp"
#include "IOEventProcessor.hpp"
#include "IODispatcherSubsystem.hpp"
#include "Reconnector.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

void
IODispatcherSubsystem::start() {
  if ( !_stateMachinesCount )
    throw smsc::util::Exception("IODispatcherSubsystem::start::: invalid configuration - stateMachinesCount = 0");

  Reconnector::getInstance().Start();

  _threadsPool.preCreateThreads(static_cast<int>(_stateMachinesCount));
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: Starting IOEventProcessors" );

  for(size_t i=0;i<_stateMachinesCount;i++) {
    IOEventProcessor *ioEventProcessor = new IOEventProcessor(_cMgr);

    smsc_log_debug(_logger, "IODispatcherSubsystem::start::: start IOEventProcessor");
    _threadsPool.startTask(ioEventProcessor);
  }
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: IOEventProcessors have started" );
}

void
IODispatcherSubsystem::stop() {
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: stopping IOEventProcessorsPool" );
  _threadsPool.shutdown();
  smsc_log_info(_logger, "IODispatcherSubsystem::start::: IOEventProcessorsPool stopped" );
}

void
IODispatcherSubsystem::initialize()
{
  smsc_log_info(_logger, "IODispatcherSubsystem::initialize::: try init MessagesFactory and Reconnector");
  MessagesFactory::init();
  Reconnector::init();
}

void
IODispatcherSubsystem::updateStateMachinesCount(unsigned new_state_machines_count)
{
  if ( new_state_machines_count <= _stateMachinesCount )
    return;

  unsigned delta = new_state_machines_count - _stateMachinesCount;
  _stateMachinesCount = new_state_machines_count;
  for(size_t i = 0; i < delta; i++) {
    IOEventProcessor *ioEventProcessor = new IOEventProcessor(_cMgr);

    smsc_log_debug(_logger, "IODispatcherSubsystem::updateStateMachinesCount::: start IOEventProcessor");
    _threadsPool.startTask(ioEventProcessor);
  }
}

}}}}
