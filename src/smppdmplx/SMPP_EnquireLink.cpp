#include "SMPP_EnquireLink.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_EnquireLink());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_EnquireLink::SMPP_EnquireLink() : SMPP_message(ENQUIRE_LINK) {}

smpp_dmplx::SMPP_EnquireLink::~SMPP_EnquireLink() {}

bool
smpp_dmplx::SMPP_EnquireLink::checkMessageCodeEquality(uint32_t msgCode) const
{
  if ( msgCode == ENQUIRE_LINK ) return true;
  else return false;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_EnquireLink::marshal() const
{
  return SMPP_message::marshal();
}

void
smpp_dmplx::SMPP_EnquireLink::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  SMPP_message::unmarshal(buf);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_EnquireLink::clone(uint32_t msgCode, BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_EnquireLink());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_EnquireLink_Resp>
smpp_dmplx::SMPP_EnquireLink::prepareResponse(uint32_t status) const
{
  std::auto_ptr<smpp_dmplx::SMPP_EnquireLink_Resp> response(new SMPP_EnquireLink_Resp());
  response->setCommandLength(SMPP_HEADER_SZ);
  response->setCommandStatus(status);
  response->setSequenceNumber(getSequenceNumber());

  return response;
}
