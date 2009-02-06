#ifndef __SMPPDMPLX_BINDRESPONSE_SUBSCRIBER_HPP__
# define __SMPPDMPLX_BINDRESPONSE_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPP_Subscriber.hpp>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>
# include <smppdmplx/SMPP_BindResponse.hpp>

namespace smpp_dmplx {

class BindResponse_Subscriber : public SMPP_Subscriber {
public:
  BindResponse_Subscriber();
  virtual ~BindResponse_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  void completeSessionActivation(SMPPSession& sessionToSmsc,
                                 SMPP_BindResponse* bindResponse);

  void terminateConnectionsPair(SMPPSession& smppSession,
                                smsc::core_ax::network::Socket& socketToSmsc,
                                const std::string& systemId);

  void forwardBindResponseMessageToSme(uint32_t originalSeqNum,
                                       SMPPSession& smppSession,
                                       SMPP_BindResponse* bindResponse,
                                       smsc::core_ax::network::Socket& socketToSmsc,
                                       const std::string& systemId);

  smsc::logger::Logger* _log;
};

}

#endif
