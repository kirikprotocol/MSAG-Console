#include "AcceptNewConnectionEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
AcceptNewConnectionEvent::handle()
{
  _switchCircuitCtrl.activateConnection(_newSocket, *_ioProcessor);
}

}}}
