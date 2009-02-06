#ifndef __SMPPDMPLX_ENQUIRELINKREQUEST_SUBSCRIBER_HPP__
# define __SMPPDMPLX_ENQUIRELINKREQUEST_SUBSCRIBER_HPP__

# include <logger/Logger.h>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/SMPP_Subscriber.hpp>
# include <smppdmplx/SMPP_EnquireLink.hpp>

namespace smpp_dmplx {

class EnquireLinkRequest_Subscriber : public SMPP_Subscriber {
public:
  EnquireLinkRequest_Subscriber();
  virtual ~EnquireLinkRequest_Subscriber();
  virtual handle_result_t handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  void forwardMessageToSmsc(SMPP_EnquireLink* enquireRequest,
                            SMPPSession& smppSessionToSme,
                            SMPPSession& sessionToSmsc);

  void broadcastMessageToAllSme(SMPP_EnquireLink* enquireRequest,
                                const std::string& systemId);

  smsc::logger::Logger* _log;
};

}

#endif
