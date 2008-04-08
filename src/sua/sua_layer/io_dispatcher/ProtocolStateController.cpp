#include "ProtocolStateController.hpp"

namespace io_dispatcher {

ProtocolStateController::ProtocolStateController(ProtocolState* initialState)
  : _protocolState(initialState) {}

void
ProtocolStateController::doStateTransition(const communication::Message& message) {
  _protocolState->checkState(this, message);
}

void
ProtocolStateController::doStateTransition(const IndicationPrimitive& indication) {
  _protocolState->checkState(this, indication);
}

void
ProtocolStateController::setState (ProtocolState* newState) { _protocolState = newState; }

void
ProtocolState::updateProtocolState(io_dispatcher::ProtocolStateController* protocolController, ProtocolState* newState)
{
  protocolController->setState(newState); 
}

}
