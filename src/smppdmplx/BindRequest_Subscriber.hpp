#ifndef __SMPPDMPLX_BINDREQUEST_SUBSCRIBER_HPP__
# define __SMPPDMPLX_BINDREQUEST_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/SMPP_Subscriber.hpp>
# include <smppdmplx/SMPP_BindRequest.hpp>
# include <smppdmplx/SMPP_Constants.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class BindRequest_Subscriber : public SMPP_Subscriber {
public:
  BindRequest_Subscriber();
  virtual ~BindRequest_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);

private:
  void initiateNewSessionToSmsc(SMPP_BindRequest* bindRequest,
                                SMPPSession& smppSessionToSME);

  void confirmIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                     SMPPSession& smppSessionToSME);

  void pendIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                  SMPPSession& smppSessionToSME);

  void teminateSession(smsc::core_ax::network::Socket& socket,
                       SMPP_BindRequest* bindRequest);

  void prepareBindResponseMessage(smsc::core_ax::network::Socket& socket,
                                  SMPP_BindRequest* bindRequest,
                                  smpp_status_t statusInResponse);

  smsc::logger::Logger* _log;
};

}

#endif
