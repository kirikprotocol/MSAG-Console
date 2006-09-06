#include "SMPP_BindTransmitter.hpp"
#include "SMPP_BindTransmitter_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransmitter());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_BindTransmitter::SMPP_BindTransmitter() : SMPP_BindRequest(BIND_TRANSMITTER) {}

smpp_dmplx::SMPP_BindTransmitter::~SMPP_BindTransmitter() {}

bool
smpp_dmplx::SMPP_BindTransmitter::checkMessageCodeEquality(uint32_t msgCode) const
{
  return msgCode == BIND_TRANSMITTER;
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_BindTransmitter::clone(uint32_t msgCode, smpp_dmplx::BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransmitter());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_BindResponse>
smpp_dmplx::SMPP_BindTransmitter::prepareResponse(uint32_t status) const
{
  std::auto_ptr<smpp_dmplx::SMPP_BindResponse> response(new SMPP_BindTransmitter_Resp());
  response->setCommandLength(response->getCommandLength() + 
                             getSystemId().length() + 1 );
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());
  response->setSystemId(getSystemId());

  return response;
}
