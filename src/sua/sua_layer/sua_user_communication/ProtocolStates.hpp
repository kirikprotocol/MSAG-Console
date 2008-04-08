#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_PROTOCOLSTATES_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_PROTOCOLSTATES_HPP__ 1

# include <sua/sua_layer/io_dispatcher/ProtocolStateController.hpp>
# include <sua/utilx/Singleton.hpp>

namespace sua_user_communication {

class LibSua_State_NoConnection : public io_dispatcher::ProtocolState,
                                  public utilx::Singleton<LibSua_State_NoConnection*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSua_State_NoConnection*>;

  LibSua_State_NoConnection();
  // disable copy
  LibSua_State_NoConnection(const LibSua_State_NoConnection& rhs);
  LibSua_State_NoConnection& operator=(const LibSua_State_NoConnection& rhs);

  uint32_t _tcpConnectEstablishedInd_MessageCode;
};

class LibSua_State_Unbind : public io_dispatcher::ProtocolState,
                            public utilx::Singleton<LibSua_State_Unbind*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSua_State_Unbind*>;

  LibSua_State_Unbind();
  // disable copy
  LibSua_State_Unbind(const LibSua_State_Unbind& rhs);
  LibSua_State_Unbind& operator=(const LibSua_State_Unbind& rhs);

  uint32_t _bindMessageCode, _tcpConnectReleasedInd_MessageCode, _MErrorMessageCode;
};

class LibSua_State_Bind : public io_dispatcher::ProtocolState,
                          public utilx::Singleton<LibSua_State_Bind*> {
public: 
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const communication::Message& message);
  virtual void checkState(io_dispatcher::ProtocolStateController* protocolController, const io_dispatcher::IndicationPrimitive& indication);
private:
  friend class utilx::Singleton<LibSua_State_Bind*>;

  LibSua_State_Bind();
  // disable copy
  LibSua_State_Bind(const LibSua_State_Bind& rhs);
  LibSua_State_Bind& operator=(const LibSua_State_Bind& rhs);

  uint32_t _unbindMessageCode, _tcpConnectReleasedInd_MessageCode;
};

}

#endif
