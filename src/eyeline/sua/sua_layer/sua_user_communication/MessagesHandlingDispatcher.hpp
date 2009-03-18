#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_MESSAGESHANDLINGDISPATCHER_HPP__

# include <eyeline/sua/communication/libsua_messages/BindMessage.hpp>
# include <eyeline/sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
# include <eyeline/sua/communication/libsua_messages/UnbindMessage.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class BindMessage_HandlingDispatcher : public communication::libsua_messages::BindMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UnbindMessage_HandlingDispatcher : public communication::libsua_messages::UnbindMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class N_UNITDATA_REQ_Message_HandlingDispatcher : public communication::libsua_messages::N_UNITDATA_REQ_Message,
                                                  public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

}}}}

#endif
