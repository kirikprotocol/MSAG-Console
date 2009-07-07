#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_PROTOCOLSTATES_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_PROTOCOLSTATES_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/prot_fsm/ProtocolStateController.hpp"
# include "eyeline/utilx/prot_fsm/TCPIndicationPrimitive.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPProtocolStateController.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SMPP_NotConnected :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_NotConnected*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_NotConnected*>;

  SMPP_NotConnected() {}
  // disable copy
  SMPP_NotConnected(const SMPP_NotConnected& rhs);
  SMPP_NotConnected& operator=(const SMPP_NotConnected& rhs);
};

class SMPP_Connected :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Connected*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Connected*>;

  SMPP_Connected() {}
  // disable copy
  SMPP_Connected(const SMPP_Connected& rhs);
  SMPP_Connected& operator=(const SMPP_Connected& rhs);
};

class SMPP_Binding_TX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Binding_TX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Binding_TX*>;

  SMPP_Binding_TX() {}
  // disable copy
  SMPP_Binding_TX(const SMPP_Binding_TX& rhs);
  SMPP_Binding_TX& operator=(const SMPP_Binding_TX& rhs);
};

class SMPP_Bound_TX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Bound_TX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Bound_TX*>;

  SMPP_Bound_TX() {}
  // disable copy
  SMPP_Bound_TX(const SMPP_Bound_TX& rhs);
  SMPP_Bound_TX& operator=(const SMPP_Bound_TX& rhs);
};

class SMPP_Binding_RX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Binding_RX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Binding_RX*>;

  SMPP_Binding_RX() {}
  // disable copy
  SMPP_Binding_RX(const SMPP_Binding_RX& rhs);
  SMPP_Binding_RX& operator=(const SMPP_Binding_RX& rhs);
};

class SMPP_Bound_RX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Bound_RX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Bound_RX*>;

  SMPP_Bound_RX() {}
  // disable copy
  SMPP_Bound_RX(const SMPP_Bound_RX& rhs);
  SMPP_Bound_RX& operator=(const SMPP_Bound_RX& rhs);
};

class SMPP_Binding_TRX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Binding_TRX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Binding_TRX*>;

  SMPP_Binding_TRX() {}
  // disable copy
  SMPP_Binding_TRX(const SMPP_Binding_TRX& rhs);
  SMPP_Binding_TRX& operator=(const SMPP_Binding_TRX& rhs);
};

class SMPP_Bound_TRX :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Bound_TRX*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Bound_TRX*>;

  SMPP_Bound_TRX() {}
  // disable copy
  SMPP_Bound_TRX(const SMPP_Bound_TRX& rhs);
  SMPP_Bound_TRX& operator=(const SMPP_Bound_TRX& rhs);
};

class SMPP_Unbound :
  public SMPPProtocolState,
  public utilx::Singleton<SMPP_Unbound*> {
public:
  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const SMPPMessage& message);

  virtual void checkState(SMPPProtocolStateController* protocol_state_controller,
                          const utilx::prot_fsm::TCPIndicationPrimitive& indication);

private:
  friend class utilx::Singleton<SMPP_Unbound*>;

  SMPP_Unbound() {}
  // disable copy
  SMPP_Unbound(const SMPP_Unbound& rhs);
  SMPP_Unbound& operator=(const SMPP_Unbound& rhs);
};

}}}}

#endif
