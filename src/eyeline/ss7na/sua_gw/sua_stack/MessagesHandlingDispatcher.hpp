#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGESHANDLINGDISPATCHER_HPP__

# include "eyeline/ss7na/common/MessageHandlingDispatcherIface.hpp"

# include "eyeline/ss7na/sua_gw/sua_stack/messages/ActiveAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ActiveMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDRMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DAVAMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DRSTMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DUNAMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DUPUMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DownAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/DownMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ErrorMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/InactiveAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/InactiveMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/NotifyMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SCONMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/UPAckMessage.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/UPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class ActiveAckMessage_HandlingDispatcher : public messages::ActiveAckMessage,
                                            public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class ActiveMessage_HandlingDispatcher : public messages::ActiveMessage,
                                         public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class CLDTMessage_HandlingDispatcher : public messages::CLDTMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class CLDRMessage_HandlingDispatcher : public messages::CLDRMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DAVAMessage_HandlingDispatcher : public messages::DAVAMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DRSTMessage_HandlingDispatcher : public messages::DRSTMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DUNAMessage_HandlingDispatcher : public messages::DUNAMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DUPUMessage_HandlingDispatcher : public messages::DUPUMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class SCONMessage_HandlingDispatcher : public messages::SCONMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DownAckMessage_HandlingDispatcher : public messages::DownAckMessage,
                                          public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class DownMessage_HandlingDispatcher : public messages::DownMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class ErrorMessage_HandlingDispatcher : public messages::ErrorMessage,
                                        public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class NotifyMessage_HandlingDispatcher : public messages::NotifyMessage,
                                         public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class InactiveAckMessage_HandlingDispatcher : public messages::InactiveAckMessage,
                                              public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class InactiveMessage_HandlingDispatcher : public messages::InactiveMessage,
                                           public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class UPAckMessage_HandlingDispatcher : public messages::UPAckMessage,
                                        public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

class UPMessage_HandlingDispatcher : public messages::UPMessage,
                                     public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

}}}}

#endif
