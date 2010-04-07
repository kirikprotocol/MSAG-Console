#ifndef __EYELINE_SS7NA_COMMON_MESSAGEHANDLINGDISPATCHERIFACE_HPP__
# define __EYELINE_SS7NA_COMMON_MESSAGEHANDLINGDISPATCHERIFACE_HPP__

# include "eyeline/ss7na/common/LinkId.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class MessageHandlingDispatcherIface {
public:
  virtual ~MessageHandlingDispatcherIface() {}
  virtual void dispatch_handle(const LinkId& linkId) const = 0;
};

}}}

#endif
