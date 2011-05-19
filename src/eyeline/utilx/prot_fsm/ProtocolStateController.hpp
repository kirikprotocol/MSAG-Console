#ifndef __EYELINE_UTILX_PROTFSM_PROTOCOLSTATECONTROLLER_HPP__
# define __EYELINE_UTILX_PROTFSM_PROTOCOLSTATECONTROLLER_HPP__

namespace eyeline {
namespace utilx {
namespace prot_fsm {

template<class MESSAGE, class NETWORK_INDICATION> class ProtocolStateController;

template<class MESSAGE, class NETWORK_INDICATION>
class ProtocolState {
public:
  virtual void checkState(ProtocolStateController<MESSAGE, NETWORK_INDICATION>* protocolStateController, const MESSAGE& message) = 0;
  virtual void checkState(ProtocolStateController<MESSAGE, NETWORK_INDICATION>* protocolStateController, const NETWORK_INDICATION& indication) = 0;
protected:
  void updateProtocolState(ProtocolStateController<MESSAGE, NETWORK_INDICATION>* protocolStateController, ProtocolState<MESSAGE, NETWORK_INDICATION>* newState);
  friend class ProtocolStateController<MESSAGE, NETWORK_INDICATION>;
};

template<class MESSAGE, class NETWORK_INDICATION>
class ProtocolStateController {
public:
  ProtocolStateController(ProtocolState<MESSAGE, NETWORK_INDICATION>* initialState);

  void doStateTransition(const MESSAGE& message);
  void doStateTransition(const NETWORK_INDICATION& indication);
private:
  typedef ProtocolState<MESSAGE, NETWORK_INDICATION> PState;
  typedef ProtocolStateController<MESSAGE, NETWORK_INDICATION> PStateCon;
  void setState(PState* newState);
  friend void PState::updateProtocolState(PStateCon* protocolStateController, PState* newState);
  //(typename ProtocolStateController<MESSAGE, NETWORK_INDICATION>* protocolStateController, typename ProtocolState<MESSAGE, NETWORK_INDICATION>* newState);

  ProtocolState<MESSAGE, NETWORK_INDICATION>* _protocolState;
};

# include <eyeline/utilx/prot_fsm/ProtocolStateController_impl.hpp>

}}}

#endif
