#include "SMPP_BindTransmitter_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransmitter_Resp());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_BindTransmitter_Resp::SMPP_BindTransmitter_Resp() : SMPP_BindResponse(BIND_TRANSMITTER_RESP) {}

smpp_dmplx::SMPP_BindTransmitter_Resp::~SMPP_BindTransmitter_Resp() {}

bool
smpp_dmplx::SMPP_BindTransmitter_Resp::checkMessageCodeEquality(uint32_t msgCode) const
{
  return msgCode == BIND_TRANSMITTER_RESP;
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_BindTransmitter_Resp::clone(uint32_t msgCode, smpp_dmplx::BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindTransmitter_Resp());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_BindResponse>
smpp_dmplx::SMPP_BindTransmitter_Resp::makeCopy() const
{
  std::auto_ptr<SMPP_BindResponse> copyMessage(new SMPP_BindTransmitter_Resp(*this));

  return copyMessage;
}
