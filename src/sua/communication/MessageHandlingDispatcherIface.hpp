#ifndef __SUA_COMMUNICATION_MESSAGEHANDLINGDISPATCHERIFACE_HPP__
# define __SUA_COMMUNICATION_MESSAGEHANDLINGDISPATCHERIFACE_HPP__ 1

# include <sua/communication/LinkId.hpp>

namespace communication {

class MessageHandlingDispatcherIface {
public:
  virtual ~MessageHandlingDispatcherIface() {}
  virtual void dispatch_handle(const LinkId& linkId) const = 0;
};

}

#endif
