#ifndef __SMPPDMPLX_ENQUIRELINKRESPONSE_SUBSCRIBER_HPP__
# define __SMPPDMPLX_ENQUIRELINKRESPONSE_SUBSCRIBER_HPP__ 1

# include "SMPP_Subscriber.hpp"

namespace smpp_dmplx {

class EnquireLinkResponse_Subscriber : public SMPP_Subscriber {
public:
  virtual ~EnquireLinkResponse_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
};

}

#endif
