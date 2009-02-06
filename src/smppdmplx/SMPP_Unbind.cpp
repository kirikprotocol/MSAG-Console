#include "SMPP_Unbind.hpp"

namespace smpp_dmplx{

SMPP_Unbind::SMPP_Unbind() : SMPP_message(UNBIND) {}

SMPP_Unbind::~SMPP_Unbind() {}

std::auto_ptr<BufferedOutputStream>
SMPP_Unbind::marshal() const
{
  return SMPP_message::marshal();
}

void
SMPP_Unbind::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<SMPP_message>
SMPP_Unbind::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_Unbind());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_Unbind_Resp>
SMPP_Unbind::prepareResponse(uint32_t status) const
{
  std::auto_ptr<SMPP_Unbind_Resp> response(new SMPP_Unbind_Resp());
  response->setCommandLength(SMPP_HEADER_SZ);
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());

  return response;
}

}
