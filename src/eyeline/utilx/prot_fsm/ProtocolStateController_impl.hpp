template<class MESSAGE, class NETWORK_INDICATION>
ProtocolStateController<MESSAGE, NETWORK_INDICATION>::ProtocolStateController(ProtocolState<MESSAGE, NETWORK_INDICATION>* initialState)
  : _protocolState(initialState)
{}

template<class MESSAGE, class NETWORK_INDICATION>
void
ProtocolStateController<MESSAGE, NETWORK_INDICATION>::doStateTransition(const MESSAGE& message) {
  _protocolState->checkState(this, message);
}

template<class MESSAGE, class NETWORK_INDICATION>
void
ProtocolStateController<MESSAGE, NETWORK_INDICATION>::doStateTransition(const NETWORK_INDICATION& indication) {
  _protocolState->checkState(this, indication);
}

template<class MESSAGE, class NETWORK_INDICATION>
void
ProtocolStateController<MESSAGE, NETWORK_INDICATION>::setState(ProtocolState<MESSAGE, NETWORK_INDICATION>* newState) {
  _protocolState = newState;
}

template<class MESSAGE, class NETWORK_INDICATION>
void
ProtocolState<MESSAGE, NETWORK_INDICATION>::updateProtocolState(ProtocolStateController<MESSAGE, NETWORK_INDICATION>* protocolController, ProtocolState<MESSAGE, NETWORK_INDICATION>* newState)
{
  protocolController->setState(newState); 
}
