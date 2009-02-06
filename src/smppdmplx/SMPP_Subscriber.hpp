#ifndef __SMPPDMPLX_SMPP_SUBSCRIBER_HPP__
# define __SMPPDMPLX_SMPP_SUBSCRIBER_HPP__

# include <memory>
# include <smppdmplx/core_ax/network/Socket.hpp>
# include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_Subscriber {
public:
  typedef enum { RequestWasProcessed = 0, RequestIsNotForMe = -1 } handle_result_t;

  virtual ~SMPP_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket) = 0;
};

}

#endif
