#include "SMPP_Unbind.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_Unbind());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_Unbind::SMPP_Unbind() : SMPP_message(UNBIND) {}

smpp_dmplx::SMPP_Unbind::~SMPP_Unbind() {}

bool
smpp_dmplx::SMPP_Unbind::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( msgCode == UNBIND ) return true;
  else return false;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_Unbind::marshal() const
{
  return SMPP_message::marshal();
}

void
smpp_dmplx::SMPP_Unbind::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_Unbind::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_Unbind());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_Unbind_Resp>
smpp_dmplx::SMPP_Unbind::prepareResponse(uint32_t status) const
{
  std::auto_ptr<smpp_dmplx::SMPP_Unbind_Resp> response(new SMPP_Unbind_Resp());
  response->setCommandLength(SMPP_HEADER_SZ);
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());

  return response;
}
