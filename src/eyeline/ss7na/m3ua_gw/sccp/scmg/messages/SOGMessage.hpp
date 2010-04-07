#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SOGMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SOGMESSAGE_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SCMGMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

class SOGMessage : public SCMGMessage {
public:
  SOGMessage()
  : SCMGMessage(_MSG_CODE)
  {}

  virtual const char* getMsgCodeTextDescription() const { return "SOG"; }

private:
  static const msg_code_t _MSG_CODE = 0x05;
};

}}}}}}

#endif
