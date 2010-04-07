#include "ProtocolStateController.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

ProtocolStateController::ProtocolStateController(ProtocolState* initial_state)
  : _protocolState(initial_state) {}

void
ProtocolStateController::doStateTransition(const Message& message) {
  _protocolState->checkState(this, message);
}

void
ProtocolStateController::doStateTransition(const IndicationPrimitive& indication) {
  _protocolState->checkState(this, indication);
}

void
ProtocolStateController::setState (ProtocolState* new_state) { _protocolState = new_state; }

void
ProtocolState::updateProtocolState(io_dispatcher::ProtocolStateController* protocol_controller,
                                   ProtocolState* new_state)
{
  protocol_controller->setState(new_state);
}

}}}}
