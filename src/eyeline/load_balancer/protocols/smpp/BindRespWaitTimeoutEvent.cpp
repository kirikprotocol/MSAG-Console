#include "BindRespWaitTimeoutEvent.hpp"
#include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"
#include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
BindRespWaitTimeoutEvent::handle()
{
  smsc_log_debug(_logger, "BindRespWaitTimeoutEvent::handle::: BindResponse waiting timeout on link with id='%s'",
                 _linkId.toString().c_str());
  if ( _ioProcessor.getBinder().processFailedBindResponse(_linkId) ) {
    io_subsystem::LinkId linkIdToSme = forwardMessageToSme(*_negativeBindResponse,
                                                           _linkId, _ioProcessor);
    io_subsystem::SwitchingTable::getInstance().removeSwitching(_linkId);
  }
  _ioProcessor.removeOutcomingLink(_linkId, true);
}

}}}}
