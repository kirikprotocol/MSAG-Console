#ifndef __SUALAYER_SUASTACK_MESSAGES_HANDLING_DISPATCHER_HPP__
# define __SUALAYER_SUASTACK_MESSAGES_HANDLING_DISPATCHER_HPP__ 1

# include <sua/communication/MessageHandlingDispatcherIface.hpp>

# include <sua/communication/sua_messages/ActiveAckMessage.hpp>
# include <sua/communication/sua_messages/ActiveMessage.hpp>
# include <sua/communication/sua_messages/CLDTMessage.hpp>
# include <sua/communication/sua_messages/DAUDMessage.hpp>
# include <sua/communication/sua_messages/DAVAMessage.hpp>
# include <sua/communication/sua_messages/DRSTMessage.hpp>
# include <sua/communication/sua_messages/DUNAMessage.hpp>
# include <sua/communication/sua_messages/DUPUMessage.hpp>
# include <sua/communication/sua_messages/DownAckMessage.hpp>
# include <sua/communication/sua_messages/DownMessage.hpp>
# include <sua/communication/sua_messages/ErrorMessage.hpp>
# include <sua/communication/sua_messages/InactiveAckMessage.hpp>
# include <sua/communication/sua_messages/InactiveMessage.hpp>
# include <sua/communication/sua_messages/NotifyMessage.hpp>
# include <sua/communication/sua_messages/SCONMessage.hpp>
# include <sua/communication/sua_messages/UPAckMessage.hpp>
# include <sua/communication/sua_messages/UPMessage.hpp>

namespace sua_stack {

class ActiveAckMessage_HandlingDispatcher : public sua_messages::ActiveAckMessage,
                                            public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class ActiveMessage_HandlingDispatcher : public sua_messages::ActiveMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class CLDTMessage_HandlingDispatcher : public sua_messages::CLDTMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DAUDMessage_HandlingDispatcher : public sua_messages::DAUDMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DAVAMessage_HandlingDispatcher : public sua_messages::DAVAMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DRSTMessage_HandlingDispatcher : public sua_messages::DRSTMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DUNAMessage_HandlingDispatcher : public sua_messages::DUNAMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DUPUMessage_HandlingDispatcher : public sua_messages::DUPUMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class SCONMessage_HandlingDispatcher : public sua_messages::SCONMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DownAckMessage_HandlingDispatcher : public sua_messages::DownAckMessage,
                                          public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class DownMessage_HandlingDispatcher : public sua_messages::DownMessage,
                                       public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class ErrorMessage_HandlingDispatcher : public sua_messages::ErrorMessage,
                                        public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class NotifyMessage_HandlingDispatcher : public sua_messages::NotifyMessage,
                                         public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class InactiveAckMessage_HandlingDispatcher : public sua_messages::InactiveAckMessage,
                                              public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class InactiveMessage_HandlingDispatcher : public sua_messages::InactiveMessage,
                                           public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UPAckMessage_HandlingDispatcher : public sua_messages::UPAckMessage,
                                        public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

class UPMessage_HandlingDispatcher : public sua_messages::UPMessage,
                                     public communication::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const communication::LinkId& linkId) const;
};

}

#endif
