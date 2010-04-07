#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_MESSAGESHANDLINGDISPATCHER_HPP__

# include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

template <class MSG_HNDLRS>
class BindMessage_HandlingDispatcher : public libsccp::BindMessage,
                                       public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const LinkId& link_id) const {
    MSG_HNDLRS msgHndlrs;
    msgHndlrs.handle(*this, link_id);
  }
};

template <class MSG_HNDLRS>
class UnbindMessage_HandlingDispatcher : public libsccp::UnbindMessage,
                                         public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const LinkId& link_id) const {
    MSG_HNDLRS msgHndlrs;
    msgHndlrs.handle(*this, link_id);
  }
};

template <class MSG_HNDLRS>
class N_UNITDATA_REQ_Message_HandlingDispatcher : public libsccp::N_UNITDATA_REQ_Message,
                                                  public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle(const LinkId& link_id) const {
    MSG_HNDLRS msgHndlrs;
    msgHndlrs.handle(*this, link_id);
  }
};

}}}}

#endif
