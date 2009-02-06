#include "SMPP_BindReceiver.hpp"
#include "SMPP_BindReceiver_Resp.hpp"

namespace smpp_dmplx {

SMPP_BindReceiver::SMPP_BindReceiver() : SMPP_BindRequest(BIND_RECEIVER) {}

SMPP_BindReceiver::~SMPP_BindReceiver() {}

std::auto_ptr<SMPP_message>
SMPP_BindReceiver::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindReceiver());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<SMPP_BindResponse>
SMPP_BindReceiver::prepareResponse(uint32_t status) const
{
  std::auto_ptr<SMPP_BindResponse> response(new SMPP_BindReceiver_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}

}
