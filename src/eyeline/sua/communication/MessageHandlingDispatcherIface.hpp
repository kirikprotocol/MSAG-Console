#ifndef __EYELINE_SUA_COMMUNICATION_MESSAGEHANDLINGDISPATCHERIFACE_HPP__
# define __EYELINE_SUA_COMMUNICATION_MESSAGEHANDLINGDISPATCHERIFACE_HPP__

# include <eyeline/sua/communication/LinkId.hpp>

namespace eyeline {
namespace sua {
namespace communication {

class MessageHandlingDispatcherIface {
public:
  virtual ~MessageHandlingDispatcherIface() {}
  virtual void dispatch_handle(const LinkId& linkId) const = 0;
};

}}}

#endif
