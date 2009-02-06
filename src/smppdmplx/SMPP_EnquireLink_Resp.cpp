#include "SMPP_EnquireLink_Resp.hpp"

namespace smpp_dmplx {

SMPP_EnquireLink_Resp::SMPP_EnquireLink_Resp() : SMPP_message(ENQUIRE_LINK_RESP) {}

SMPP_EnquireLink_Resp::~SMPP_EnquireLink_Resp() {}

std::auto_ptr<BufferedOutputStream>
SMPP_EnquireLink_Resp::marshal() const
{
  return SMPP_message::marshal();
}

void
SMPP_EnquireLink_Resp::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<SMPP_message>
SMPP_EnquireLink_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_EnquireLink_Resp());
  message->unmarshal(buf);

  return message;
}

}
