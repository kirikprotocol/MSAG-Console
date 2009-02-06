#ifndef __SMPPDMPLX_GENERICRESPONSE_SUBSCRIBER_HPP__
# define __SMPPDMPLX_GENERICRESPONSE_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPP_Subscriber.hpp>
# include <smppdmplx/SMPP_GenericResponse.hpp>
# include <smppdmplx/SMPPSession.hpp>

namespace smpp_dmplx {

class GenericResponse_Subscriber : public SMPP_Subscriber {
public:
  GenericResponse_Subscriber();
  virtual ~GenericResponse_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  void forwardMessageToSmsc(SMPP_GenericResponse* genericResponse,
                            SMPPSession& smppSessionToSme,
                            SMPPSession& sessionToSMSC);

  void forwardMessageToSme(SMPP_GenericResponse* genericResponse,
                           SMPPSession& sessionToSmsc);

  smsc::logger::Logger* _log;
};

}

#endif

