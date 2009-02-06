#ifndef __SMPPDMPLX_GENERICREQUEST_SUBSCRIBER_HPP__
# define __SMPPDMPLX_GENERICREQUEST_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPP_Subscriber.hpp>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/SMPP_GenericRequest.hpp>

namespace smpp_dmplx {

class GenericRequest_Subscriber : public SMPP_Subscriber {
public:
  GenericRequest_Subscriber();
  virtual ~GenericRequest_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  void forwardMessageToSmsc(SMPP_GenericRequest* genericRequest,
                            SMPPSession& smppSessionToSme,
                            SMPPSession& sessionToSMSC);

  void forwardMessageToSme(SMPP_GenericRequest* genericRequest,
                           SMPPSession& sessionToSMSC);

  smsc::logger::Logger* _log;
};

}

#endif
