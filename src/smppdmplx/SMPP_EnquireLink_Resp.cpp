#include "SMPP_EnquireLink_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_EnquireLink_Resp());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_EnquireLink_Resp::SMPP_EnquireLink_Resp() : SMPP_message(ENQUIRE_LINK_RESP) {}

smpp_dmplx::SMPP_EnquireLink_Resp::~SMPP_EnquireLink_Resp() {}

bool
smpp_dmplx::SMPP_EnquireLink_Resp::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( msgCode == ENQUIRE_LINK_RESP ) return true;
  else return false;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_EnquireLink_Resp::marshal() const
{
  return SMPP_message::marshal();
}

void
smpp_dmplx::SMPP_EnquireLink_Resp::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_EnquireLink_Resp::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_EnquireLink_Resp());
  message->unmarshal(buf);

  return message;
}
