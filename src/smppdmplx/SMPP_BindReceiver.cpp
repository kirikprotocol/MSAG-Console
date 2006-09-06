#include "SMPP_BindReceiver.hpp"
#include "SMPP_BindReceiver_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindReceiver());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_BindReceiver::SMPP_BindReceiver() : SMPP_BindRequest(BIND_RECEIVER) {}

smpp_dmplx::SMPP_BindReceiver::~SMPP_BindReceiver() {}

bool
smpp_dmplx::SMPP_BindReceiver::checkMessageCodeEquality(uint32_t msgCode) const
{
  return msgCode == BIND_RECEIVER;
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_BindReceiver::clone(uint32_t msgCode, smpp_dmplx::BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindReceiver());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_BindResponse>
smpp_dmplx::SMPP_BindReceiver::prepareResponse(uint32_t status) const
{
  std::auto_ptr<smpp_dmplx::SMPP_BindResponse> response(new SMPP_BindReceiver_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}
