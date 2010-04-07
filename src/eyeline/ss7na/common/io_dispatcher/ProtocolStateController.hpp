#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_PROTOCOLSTATECONTROLLER_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_PROTOCOLSTATECONTROLLER_HPP__

# include "eyeline/ss7na/common/io_dispatcher/IndicationPrimitive.hpp"
# include "eyeline/ss7na/common/Message.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ProtocolStateController;

class ProtocolState {
public:
  virtual void checkState(ProtocolStateController* protocol_state_controller,
                          const Message& message) = 0;
  virtual void checkState(ProtocolStateController* protocol_state_controller,
                          const IndicationPrimitive& indication) = 0;
protected:
  void updateProtocolState(ProtocolStateController* protocol_state_controller,
                           ProtocolState* new_state);
  friend class ProtocolStateController;
};

class ProtocolStateController {
public:
  ProtocolStateController(ProtocolState* initial_state);

  void doStateTransition(const Message& message);
  void doStateTransition(const IndicationPrimitive& indication);
private:
  void setState (ProtocolState* new_state);
  friend void ProtocolState::updateProtocolState(ProtocolStateController* protocol_state_controller,
                                                 ProtocolState* new_state);

  ProtocolState* _protocolState;
};

}}}}

#endif
