#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_LUDTS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_LUDTS_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class LUDTS : public SCCPMessage {
public:
  LUDTS()
  : SCCPMessage(_MSG_CODE) {}

private:
  static const msg_code_t _MSG_CODE = 0x14;
};

}}}}}

#endif
