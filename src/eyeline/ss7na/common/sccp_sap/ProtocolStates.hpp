#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_PROTOCOLSTATES_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_PROTOCOLSTATES_HPP__

# include "eyeline/ss7na/common/io_dispatcher/ProtocolStateController.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class LibSccp_State_NoConnection : public io_dispatcher::ProtocolState,
                                  public utilx::Singleton<LibSccp_State_NoConnection*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSccp_State_NoConnection*>;

  LibSccp_State_NoConnection();
  // disable copy
  LibSccp_State_NoConnection(const LibSccp_State_NoConnection& rhs);
  LibSccp_State_NoConnection& operator=(const LibSccp_State_NoConnection& rhs);

  uint32_t _tcpConnectEstablishedInd_MessageCode;
};

class LibSccp_State_Unbind : public io_dispatcher::ProtocolState,
                            public utilx::Singleton<LibSccp_State_Unbind*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSccp_State_Unbind*>;

  LibSccp_State_Unbind();
  // disable copy
  LibSccp_State_Unbind(const LibSccp_State_Unbind& rhs);
  LibSccp_State_Unbind& operator=(const LibSccp_State_Unbind& rhs);

  uint32_t _bindMessageCode, _tcpConnectReleasedInd_MessageCode;
};

class LibSccp_State_Bind : public io_dispatcher::ProtocolState,
                          public utilx::Singleton<LibSccp_State_Bind*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocol_controller,
                          const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSccp_State_Bind*>;

  LibSccp_State_Bind();
  // disable copy
  LibSccp_State_Bind(const LibSccp_State_Bind& rhs);
  LibSccp_State_Bind& operator=(const LibSccp_State_Bind& rhs);

  uint32_t _unbindMessageCode, _tcpConnectReleasedInd_MessageCode;
};

}}}}

#endif
