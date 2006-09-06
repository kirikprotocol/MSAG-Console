#include "SMPP_BindReceiver_Resp.hpp"
#include "SMPP_MessageFactory.hpp"

static int toRegisterMessageInFactory() {
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindReceiver_Resp());
  return 0;
}

static int messageInFactoryIsRegistred = toRegisterMessageInFactory();

smpp_dmplx::SMPP_BindReceiver_Resp::SMPP_BindReceiver_Resp() : SMPP_BindResponse(BIND_RECEIVER_RESP) {}

smpp_dmplx::SMPP_BindReceiver_Resp::~SMPP_BindReceiver_Resp() {}

bool
smpp_dmplx::SMPP_BindReceiver_Resp::checkMessageCodeEquality(uint32_t msgCode) const
{
  return msgCode == BIND_RECEIVER_RESP;
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_BindReceiver_Resp::clone(uint32_t msgCode, smpp_dmplx::BufferedInputStream& buf) const
{
  std::auto_ptr<SMPP_message> message(new SMPP_BindReceiver_Resp());
  message->unmarshal(buf);

  return message;
}

std::auto_ptr<smpp_dmplx::SMPP_BindResponse>
smpp_dmplx::SMPP_BindReceiver_Resp::makeCopy() const
{
  std::auto_ptr<SMPP_BindResponse> copyMessage(new SMPP_BindReceiver_Resp(*this));

  return copyMessage;
}
