#ifndef __SUALAYER_SUAUSERCOMMUNICATION_MESSAGES_HANDLING_DISPATCHER_HPP__
# define __SUALAYER_SUAUSERCOMMUNICATION_MESSAGES_HANDLING_DISPATCHER_HPP__ 1

# include <sua/communication/libsua_messages/BindMessage.hpp>
# include <sua/communication/libsua_messages/EncapsulatedSuaMessage.hpp>
# include <sua/communication/libsua_messages/MErrorMessage.hpp>
# include <sua/communication/libsua_messages/UnbindMessage.hpp>

namespace sua_user_communication {

class BindMessage_HandlingDispatcher : public libsua_messages::BindMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UnbindMessage_HandlingDispatcher : public libsua_messages::UnbindMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class MErrorMessage_HandlingDispatcher : public libsua_messages::MErrorMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class EncapsulatedSuaMessage_HandlingDispatcher : public libsua_messages::EncapsulatedSuaMessage,
                                                  public communication::MessageHandlingDispatcherIface {
public:
  EncapsulatedSuaMessage_HandlingDispatcher(uint32_t msgCode)
    : libsua_messages::EncapsulatedSuaMessage(msgCode), communication::MessageHandlingDispatcherIface() {}

  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

}
#endif

