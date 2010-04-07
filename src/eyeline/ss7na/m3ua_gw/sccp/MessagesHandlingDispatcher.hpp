#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESHANDLINGDISPATCHER_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESHANDLINGDISPATCHER_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDTS.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDTS.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlingDispatcherIface.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class UDT_HandlingDispatcher : public messages::UDT,
                               public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class UDTS_HandlingDispatcher : public messages::UDTS,
                                public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class XUDT_HandlingDispatcher : public messages::XUDT,
                                public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

class XUDTS_HandlingDispatcher : public messages::XUDTS,
                                 public MessageHandlingDispatcherIface {
public:
  virtual MessageHandlingDispatcherIface* getHandlingDispatcher() {
    return this;
  }

  virtual void dispatch_handle();
};

}}}}

#endif
