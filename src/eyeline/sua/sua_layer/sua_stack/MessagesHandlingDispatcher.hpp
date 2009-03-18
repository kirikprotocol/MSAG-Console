#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_MESSAGESHANDLINGDISPATCHER_HPP__

# include <eyeline/sua/communication/MessageHandlingDispatcherIface.hpp>

# include <eyeline/sua/communication/sua_messages/ActiveAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/ActiveMessage.hpp>
# include <eyeline/sua/communication/sua_messages/CLDTMessage.hpp>
# include <eyeline/sua/communication/sua_messages/CLDRMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DAVAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DRSTMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DUNAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DUPUMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DownAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/DownMessage.hpp>
# include <eyeline/sua/communication/sua_messages/ErrorMessage.hpp>
# include <eyeline/sua/communication/sua_messages/InactiveAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/InactiveMessage.hpp>
# include <eyeline/sua/communication/sua_messages/NotifyMessage.hpp>
# include <eyeline/sua/communication/sua_messages/SCONMessage.hpp>
# include <eyeline/sua/communication/sua_messages/UPAckMessage.hpp>
# include <eyeline/sua/communication/sua_messages/UPMessage.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class ActiveAckMessage_HandlingDispatcher : public communication::sua_messages::ActiveAckMessage,
                                            public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class ActiveMessage_HandlingDispatcher : public communication::sua_messages::ActiveMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class CLDTMessage_HandlingDispatcher : public communication::sua_messages::CLDTMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class CLDRMessage_HandlingDispatcher : public communication::sua_messages::CLDRMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DAVAMessage_HandlingDispatcher : public communication::sua_messages::DAVAMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DRSTMessage_HandlingDispatcher : public communication::sua_messages::DRSTMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DUNAMessage_HandlingDispatcher : public communication::sua_messages::DUNAMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DUPUMessage_HandlingDispatcher : public communication::sua_messages::DUPUMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class SCONMessage_HandlingDispatcher : public communication::sua_messages::SCONMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DownAckMessage_HandlingDispatcher : public communication::sua_messages::DownAckMessage,
                                          public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DownMessage_HandlingDispatcher : public communication::sua_messages::DownMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class ErrorMessage_HandlingDispatcher : public communication::sua_messages::ErrorMessage,
                                        public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class NotifyMessage_HandlingDispatcher : public communication::sua_messages::NotifyMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class InactiveAckMessage_HandlingDispatcher : public communication::sua_messages::InactiveAckMessage,
                                              public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class InactiveMessage_HandlingDispatcher : public communication::sua_messages::InactiveMessage,
                                           public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UPAckMessage_HandlingDispatcher : public communication::sua_messages::UPAckMessage,
                                        public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UPMessage_HandlingDispatcher : public communication::sua_messages::UPMessage,
                                     public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

}}}}

#endif
