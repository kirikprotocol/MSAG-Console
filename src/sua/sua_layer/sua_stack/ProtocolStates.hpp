#ifndef __SUA_SUALAYER_SUSTACK_PROTOCOLSTATES_HPP__
# define __SUA_SUALAYER_SUSTACK_PROTOCOLSTATES_HPP__ 1

# include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
# include <sua/utilx/Singleton.hpp>

namespace sua_stack {

// this is initial state
class SUA_State_NoConnection : public io_dispatcher::ProtocolState,
                               public utilx::Singleton<SUA_State_NoConnection*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_NoConnection*>;

  SUA_State_NoConnection();
  // disable copy
  SUA_State_NoConnection(const SUA_State_NoConnection& rhs);
  SUA_State_NoConnection& operator=(const SUA_State_NoConnection& rhs);

  uint32_t _sctpConnectEstablishedInd_MessageCode;
};

// this state is actual when SCTP-connection has been established
class SUA_State_ASPDown : public io_dispatcher::ProtocolState,
                          public utilx::Singleton<SUA_State_ASPDown*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPDown*>;

  SUA_State_ASPDown();
  // disable copy
  SUA_State_ASPDown(const SUA_State_ASPDown& rhs);
  SUA_State_ASPDown& operator=(const SUA_State_ASPDown& rhs);

  uint32_t _upMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// this is state when DOWN message was sent but DOWN ACK message wasn't received
class SUA_State_ASPDownPending : public io_dispatcher::ProtocolState,
                                 public utilx::Singleton<SUA_State_ASPDownPending*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPDownPending*>;

  SUA_State_ASPDownPending();
  // disable copy
  SUA_State_ASPDownPending(const SUA_State_ASPDownPending& rhs);
  SUA_State_ASPDownPending& operator=(const SUA_State_ASPDownPending& rhs);

  uint32_t _downAckMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// this state is actual when ASPUP message was sent but ASPUP-Ack messages wasn't received
class SUA_State_ASPInactivePending : public io_dispatcher::ProtocolState,
                                     public utilx::Singleton<SUA_State_ASPInactivePending*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPInactivePending*>;

  SUA_State_ASPInactivePending();
  // disable copy
  SUA_State_ASPInactivePending(const SUA_State_ASPInactivePending& rhs);
  SUA_State_ASPInactivePending& operator=(const SUA_State_ASPInactivePending& rhs);

  uint32_t _upMessageAckCode, _errorMessageCode, _sctpConnectReleasedInd_MessageCode;
};


// this state is actual when got ASPUP-Ack message in response to sending ASPUP message,
// or was received INACTIVE-ACK message in response to sending INACTIVE messages
class SUA_State_ASPInactive : public io_dispatcher::ProtocolState,
                              public utilx::Singleton<SUA_State_ASPInactive*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPInactive*>;

  SUA_State_ASPInactive();
  // disable copy
  SUA_State_ASPInactive(const SUA_State_ASPInactive& rhs);
  SUA_State_ASPInactive& operator=(const SUA_State_ASPInactive& rhs);

  uint32_t _activeMessageCode, _downMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// this state is actual when ACTIVE message was sent but ACTIVE-ACK message wasn't received
class SUA_State_ASPActivePending : public io_dispatcher::ProtocolState,
                                   public utilx::Singleton<SUA_State_ASPActivePending*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPActivePending*>;

  SUA_State_ASPActivePending();
  // disable copy
  SUA_State_ASPActivePending(const SUA_State_ASPActivePending& rhs);
  SUA_State_ASPActivePending& operator=(const SUA_State_ASPActivePending& rhs);

  uint32_t _activeAckMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// ACTIVE ACK message was received
class SUA_State_ASPActive : public io_dispatcher::ProtocolState,
                            public utilx::Singleton<SUA_State_ASPActive*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPActive*>;

  SUA_State_ASPActive();
  // disable copy
  SUA_State_ASPActive(const SUA_State_ASPActive& rhs);
  SUA_State_ASPActive& operator=(const SUA_State_ASPActive& rhs);

  uint32_t _inactiveMessageCode, _downMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// in ACTIVE state ASP has sent INACTIVE message and is waiting for INACTIVE ACK message
class SUA_State_ASPActiveShutdown : public io_dispatcher::ProtocolState,
                                    public utilx::Singleton<SUA_State_ASPActiveShutdown*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolStateController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<SUA_State_ASPActiveShutdown*>;

  SUA_State_ASPActiveShutdown();
  // disable copy
  SUA_State_ASPActiveShutdown(const SUA_State_ASPActiveShutdown& rhs);
  SUA_State_ASPActiveShutdown& operator=(const SUA_State_ASPActiveShutdown& rhs);

  uint32_t _inactiveAckMessageCode, _sctpConnectReleasedInd_MessageCode;
};

template<class T>
T* utilx::Singleton<T*>::_instance;

}

#endif
