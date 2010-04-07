#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_PROTOCOLSTATES_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_PROTOCOLSTATES_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

// this is initial state
class M3UA_State_NoConnection : public common::io_dispatcher::ProtocolState,
                                public utilx::Singleton<M3UA_State_NoConnection*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_NoConnection*>;

  M3UA_State_NoConnection();
  // disable copy
  M3UA_State_NoConnection(const M3UA_State_NoConnection& rhs);
  M3UA_State_NoConnection& operator=(const M3UA_State_NoConnection& rhs);

  uint32_t _sctpConnectEstablishedInd_MessageCode;
};

// this state is actual when SCTP-connection has been established
class M3UA_State_ASPDown : public common::io_dispatcher::ProtocolState,
                           public utilx::Singleton<M3UA_State_ASPDown*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPDown*>;

  M3UA_State_ASPDown();
  // disable copy
  M3UA_State_ASPDown(const M3UA_State_ASPDown& rhs);
  M3UA_State_ASPDown& operator=(const M3UA_State_ASPDown& rhs);

  uint32_t _upMessageCode, _sctpConnectReleasedInd_MessageCode;
};

class M3UA_Incoming_ManagmentMessages_Permitted : public common::io_dispatcher::ProtocolState {
protected:
  M3UA_Incoming_ManagmentMessages_Permitted();

  void checkIfGotManagementMessage(const common::Message& message, const std::string& where);
private:
  uint32_t _notifyMessageCode, _errorMessageCode;
};


// this is state when DOWN message was sent but DOWN ACK message wasn't received
class M3UA_State_ASPDownPending : public M3UA_Incoming_ManagmentMessages_Permitted,
                                  public utilx::Singleton<M3UA_State_ASPDownPending*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPDownPending*>;

  M3UA_State_ASPDownPending();
  // disable copy
  M3UA_State_ASPDownPending(const M3UA_State_ASPDownPending& rhs);
  M3UA_State_ASPDownPending& operator=(const M3UA_State_ASPDownPending& rhs);

  uint32_t _downAckMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// this state is actual when ASPUP message was sent but ASPUP-Ack messages wasn't received
class M3UA_State_ASPInactivePending : public M3UA_Incoming_ManagmentMessages_Permitted,
                                      public utilx::Singleton<M3UA_State_ASPInactivePending*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPInactivePending*>;

  M3UA_State_ASPInactivePending();
  // disable copy
  M3UA_State_ASPInactivePending(const M3UA_State_ASPInactivePending& rhs);
  M3UA_State_ASPInactivePending& operator=(const M3UA_State_ASPInactivePending& rhs);

  uint32_t _upMessageAckCode, _errorMessageCode, _sctpConnectReleasedInd_MessageCode;
};


// this state is actual when got ASPUP-Ack message in response to sending ASPUP message,
// or was received INACTIVE-ACK message in response to sending INACTIVE messages
class M3UA_State_ASPInactive : public M3UA_Incoming_ManagmentMessages_Permitted,
                               public utilx::Singleton<M3UA_State_ASPInactive*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPInactive*>;

  M3UA_State_ASPInactive();
  // disable copy
  M3UA_State_ASPInactive(const M3UA_State_ASPInactive& rhs);
  M3UA_State_ASPInactive& operator=(const M3UA_State_ASPInactive& rhs);

  uint32_t _activeMessageCode, _downMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// this state is actual when ACTIVE message was sent but ACTIVE-ACK message wasn't received
class M3UA_State_ASPActivePending : public M3UA_Incoming_ManagmentMessages_Permitted,
                                    public utilx::Singleton<M3UA_State_ASPActivePending*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPActivePending*>;

  M3UA_State_ASPActivePending();
  // disable copy
  M3UA_State_ASPActivePending(const M3UA_State_ASPActivePending& rhs);
  M3UA_State_ASPActivePending& operator=(const M3UA_State_ASPActivePending& rhs);

  uint32_t _activeAckMessageCode, _sctpConnectReleasedInd_MessageCode;
  uint32_t _downMessageCode;
};

// ACTIVE ACK message was received
class M3UA_State_ASPActive : public common::io_dispatcher::ProtocolState,
                             public utilx::Singleton<M3UA_State_ASPActive*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPActive*>;

  M3UA_State_ASPActive();
  // disable copy
  M3UA_State_ASPActive(const M3UA_State_ASPActive& rhs);
  M3UA_State_ASPActive& operator=(const M3UA_State_ASPActive& rhs);

  uint32_t _inactiveMessageCode, _downMessageCode, _sctpConnectReleasedInd_MessageCode;
};

// in ACTIVE state ASP has sent INACTIVE message and is waiting for INACTIVE ACK message
class M3UA_State_ASPActiveShutdown : public M3UA_Incoming_ManagmentMessages_Permitted,
                                     public utilx::Singleton<M3UA_State_ASPActiveShutdown*> {
public: 
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolController,
                          const common::Message& message);
  virtual void checkState(common::io_dispatcher::ProtocolStateController* protocolStateController,
                          const common::io_dispatcher::IndicationPrimitive& indication);
private:
  using common::io_dispatcher::ProtocolState::checkState;
  friend class utilx::Singleton<M3UA_State_ASPActiveShutdown*>;

  M3UA_State_ASPActiveShutdown();
  // disable copy
  M3UA_State_ASPActiveShutdown(const M3UA_State_ASPActiveShutdown& rhs);
  M3UA_State_ASPActiveShutdown& operator=(const M3UA_State_ASPActiveShutdown& rhs);

  uint32_t _inactiveAckMessageCode, _sctpConnectReleasedInd_MessageCode;
};

}}}}}

#endif
