#include "SMPP_BindTransmitter_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindTransmitter_Resp::SMPP_BindTransmitter_Resp() : SMPP_BindResponse(BIND_TRANSMITTER_RESP) {}

SMPP_BindTransmitter_Resp::~SMPP_BindTransmitter_Resp() {}

std::auto_ptr<SMPP_message>
SMPP_BindTransmitter_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransmitter_Resp());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindTransmitter_Resp::makeCopy() const
{
  std::auto_ptr<SMPP_BindResponse> copyMessage(new SMPP_BindTransmitter_Resp(*this));

  return copyMessage;
}

}
