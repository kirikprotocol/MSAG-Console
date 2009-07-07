#include "UnbindRespWaitTimeoutEvent.hpp"
#include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
UnbindRespWaitTimeoutEvent::handle()
{
  io_subsystem::LinkId linksetId = _ioProcessor.removeOutcomingLink(_linkId, true);
  io_subsystem::LinkId emptyLinkId;
  if ( linksetId != emptyLinkId )
    _ioProcessor.removeLinkSet(linksetId);
}

}}}}
