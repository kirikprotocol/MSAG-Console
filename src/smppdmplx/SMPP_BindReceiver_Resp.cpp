#include "SMPP_BindReceiver_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindReceiver_Resp::SMPP_BindReceiver_Resp() : SMPP_BindResponse(BIND_RECEIVER_RESP) {}

SMPP_BindReceiver_Resp::~SMPP_BindReceiver_Resp() {}

std::auto_ptr<SMPP_message>
SMPP_BindReceiver_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindReceiver_Resp());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindReceiver_Resp::makeCopy() const
{
  std::auto_ptr<SMPP_BindResponse> copyMessage(new SMPP_BindReceiver_Resp(*this));

  return copyMessage;
}

}
