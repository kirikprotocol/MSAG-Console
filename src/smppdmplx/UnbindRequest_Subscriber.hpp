#ifndef __SMPPDMPLX_UNBINDREQUEST_SUBSCRIBER_HPP__
# define __SMPPDMPLX_UNBINDREQUEST_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPP_Subscriber.hpp>

namespace smpp_dmplx {

class UnbindRequest_Subscriber : public SMPP_Subscriber {
public:
  UnbindRequest_Subscriber();

  virtual ~UnbindRequest_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  smsc::logger::Logger* _log;
};

}

#endif
