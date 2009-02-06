#include "SMPP_Unbind_Resp.hpp"

namespace smpp_dmplx {

SMPP_Unbind_Resp::SMPP_Unbind_Resp() : SMPP_message(UNBIND_RESP) {}

SMPP_Unbind_Resp::~SMPP_Unbind_Resp() {}

std::auto_ptr<BufferedOutputStream>
SMPP_Unbind_Resp::marshal() const
{
  return SMPP_message::marshal();
}

void
SMPP_Unbind_Resp::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<SMPP_message>
SMPP_Unbind_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_Unbind_Resp());
  message->unmarshal(buf);

  return message;
}

}
