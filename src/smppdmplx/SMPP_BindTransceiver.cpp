#include "SMPP_BindTransceiver.hpp"
#include "SMPP_BindTransceiver_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindTransceiver::SMPP_BindTransceiver() : SMPP_BindRequest(BIND_TRANSCEIVER) {}

SMPP_BindTransceiver::~SMPP_BindTransceiver() {}

std::auto_ptr<SMPP_message>
SMPP_BindTransceiver::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransceiver());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindTransceiver::prepareResponse(uint32_t status) const
{
  std::auto_ptr<SMPP_BindResponse> response(new SMPP_BindTransceiver_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}

}
