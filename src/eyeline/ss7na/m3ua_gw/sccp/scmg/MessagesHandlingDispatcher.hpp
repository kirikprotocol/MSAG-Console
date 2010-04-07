#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESHANDLINGDISPATCHER_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlingDispatcherIface.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SOGMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SORMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSCMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

class SOGMessage_HandlingDispatcher : public messages::SOGMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class SORMessage_HandlingDispatcher : public messages::SORMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class SSAMessage_HandlingDispatcher : public messages::SSAMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class SSCMessage_HandlingDispatcher : public messages::SSCMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class SSPMessage_HandlingDispatcher : public messages::SSPMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class SSTMessage_HandlingDispatcher : public messages::SSTMessage,
                                      public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

}}}}}

#endif
