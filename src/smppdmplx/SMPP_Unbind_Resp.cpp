#include "SMPP_Unbind_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_Unbind_Resp());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_Unbind_Resp::SMPP_Unbind_Resp() : SMPP_message(UNBIND_RESP) {}

smpp_dmplx::SMPP_Unbind_Resp::~SMPP_Unbind_Resp() {}

bool
smpp_dmplx::SMPP_Unbind_Resp::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( msgCode == UNBIND_RESP ) return true;
  else return false;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_Unbind_Resp::marshal() const
{
  return SMPP_message::marshal();
}

void
smpp_dmplx::SMPP_Unbind_Resp::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_Unbind_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_Unbind_Resp());
  message->unmarshal(buf);

  return message;
}
