#include "SMPP_BindTransmitter.hpp"
#include "SMPP_BindTransmitter_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindTransmitter::SMPP_BindTransmitter() : SMPP_BindRequest(BIND_TRANSMITTER) {}

SMPP_BindTransmitter::~SMPP_BindTransmitter() {}

std::auto_ptr<SMPP_message>
SMPP_BindTransmitter::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransmitter());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindTransmitter::prepareResponse(uint32_t status) const
{
  std::auto_ptr<SMPP_BindResponse> response(new SMPP_BindTransmitter_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}

}
