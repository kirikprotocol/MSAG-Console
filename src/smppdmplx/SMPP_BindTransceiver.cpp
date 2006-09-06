#include "SMPP_BindTransceiver.hpp"
#include "SMPP_BindTransceiver_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransceiver());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_BindTransceiver::SMPP_BindTransceiver() : SMPP_BindRequest(BIND_TRANSCEIVER) {}

smpp_dmplx::SMPP_BindTransceiver::~SMPP_BindTransceiver() {}

bool
smpp_dmplx::SMPP_BindTransceiver::checkMessageCodeEquality(uint32_t msgCode) const
{
  return msgCode == BIND_TRANSCEIVER;
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_BindTransceiver::clone(uint32_t msgCode, smpp_dmplx::BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransceiver());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_BindResponse>
smpp_dmplx::SMPP_BindTransceiver::prepareResponse(uint32_t status) const
{
  std::auto_ptr<smpp_dmplx::SMPP_BindResponse> response(new SMPP_BindTransceiver_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}

