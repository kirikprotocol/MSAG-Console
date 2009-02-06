#ifndef __SMPPDMPLX_ENQUIRELINKRESPONSE_SUBSCRIBER_HPP__
# define __SMPPDMPLX_ENQUIRELINKRESPONSE_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPP_Subscriber.hpp>

namespace smpp_dmplx {

class EnquireLinkResponse_Subscriber : public SMPP_Subscriber {
public:
  EnquireLinkResponse_Subscriber();
  virtual ~EnquireLinkResponse_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  smsc::logger::Logger* _log;
};

}

#endif
