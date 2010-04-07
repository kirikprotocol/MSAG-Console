#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGEHANDLINGDISPATCHERIFACE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGEHANDLINGDISPATCHERIFACE_HPP__

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class MessageHandlingDispatcherIface {
public:
  virtual ~MessageHandlingDispatcherIface() {}
  virtual void dispatch_handle() = 0;
};

}}}}

#endif
