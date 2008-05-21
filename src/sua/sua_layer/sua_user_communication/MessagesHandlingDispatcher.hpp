#ifndef __SUALAYER_SUAUSERCOMMUNICATION_MESSAGES_HANDLING_DISPATCHER_HPP__
# define __SUALAYER_SUAUSERCOMMUNICATION_MESSAGES_HANDLING_DISPATCHER_HPP__ 1

# include <sua/communication/libsua_messages/BindMessage.hpp>
# include <sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
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

class N_UNITDATA_REQ_Message_HandlingDispatcher : public libsua_messages::N_UNITDATA_REQ_Message,
                                                  public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

}
#endif

