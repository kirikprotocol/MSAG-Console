#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SSPMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SSPMESSAGE_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SCMGMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

class SSPMessage : public SCMGMessage {
public:
  SSPMessage()
  : SCMGMessage(_MSG_CODE)
  {}

  virtual const char* getMsgCodeTextDescription() const { return "SSP"; }

private:
  static const msg_code_t _MSG_CODE = 0x02;
};

}}}}}}

#endif
