#include "SMPP_BindTransceiver_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindTransceiver_Resp::SMPP_BindTransceiver_Resp() : SMPP_BindResponse(BIND_TRANSCEIVER_RESP) {}

SMPP_BindTransceiver_Resp::~SMPP_BindTransceiver_Resp() {}

std::auto_ptr<SMPP_message>
SMPP_BindTransceiver_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransceiver_Resp());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindTransceiver_Resp::makeCopy() const
{
  std::auto_ptr<SMPP_BindResponse> copyMessage(new SMPP_BindTransceiver_Resp(*this));

  return copyMessage;
}

}
