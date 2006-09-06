#ifndef __SMPPDMPLX_GENERICRESPONSE_SUBSCRIBER_HPP__
# define __SMPPDMPLX_GENERICRESPONSE_SUBSCRIBER_HPP__ 1

# include "SMPP_Subscriber.hpp"

namespace smpp_dmplx {

class GenericResponse_Subscriber : public SMPP_Subscriber {
public:
  virtual ~GenericResponse_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
};

}

#endif

