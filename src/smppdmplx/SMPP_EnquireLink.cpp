#include "SMPP_EnquireLink.hpp"

namespace smpp_dmplx {

SMPP_EnquireLink::SMPP_EnquireLink() : SMPP_message(ENQUIRE_LINK) {}

SMPP_EnquireLink::~SMPP_EnquireLink() {}

std::auto_ptr<BufferedOutputStream>
SMPP_EnquireLink::marshal() const
{
  return SMPP_message::marshal();
}

void
SMPP_EnquireLink::unmarshal(BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<SMPP_message>
SMPP_EnquireLink::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_EnquireLink());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_EnquireLink_Resp>
SMPP_EnquireLink::prepareResponse(uint32_t status) const
{
  std::auto_ptr<SMPP_EnquireLink_Resp> response(new SMPP_EnquireLink_Resp());
  response->setCommandLength(SMPP_HEADER_SZ);
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());

  return response;
}

}
