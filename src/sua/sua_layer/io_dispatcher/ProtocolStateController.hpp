#ifndef __IO_DISPATCHER_PROTOCOLSTATECONTROLLER_HPP__
# define __IO_DISPATCHER_PROTOCOLSTATECONTROLLER_HPP__ 1

# include <sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>
# include <sua/communication/Message.hpp>

namespace io_dispatcher {

class ProtocolStateController;

class ProtocolState {
public:
  virtual void checkState(ProtocolStateController* protocolStateController, const communication::Message& message) = 0;
  virtual void checkState(ProtocolStateController* protocolStateController, const IndicationPrimitive& indication) = 0;
protected:
  void updateProtocolState(ProtocolStateController* protocolStateController, ProtocolState* newState);
  friend class ProtocolStateController;
};

class ProtocolStateController {
public:
  ProtocolStateController(ProtocolState* initialState);

  void doStateTransition(const communication::Message& message);
  void doStateTransition(const IndicationPrimitive& indication);
private:
  void setState (ProtocolState* newState);
  friend void ProtocolState::updateProtocolState(ProtocolStateController* protocolStateController, ProtocolState* newState);

  ProtocolState* _protocolState;
};

}

#endif
