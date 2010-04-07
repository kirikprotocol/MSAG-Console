#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGESHANDLINGDISPATCHER_HPP__

# include "eyeline/ss7na/common/MessageHandlingDispatcherIface.hpp"

# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ActiveAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ActiveMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DAVAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DRSTMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DUNAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DUPUMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DAUDMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DownAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DownMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/ErrorMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/InactiveAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/InactiveMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/NotifyMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/SCONMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/UPAckMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/UPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/DATAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {

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

class DAUDMessage_HandlingDispatcher : public messages::DAUDMessage,
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

class DATAMessage_HandlingDispatcher : public messages::DATAMessage,
                                       public common::MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const common::LinkId& link_id) const;
};

}}}}}

#endif
