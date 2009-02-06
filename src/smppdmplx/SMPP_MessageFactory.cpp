#include <utility>
#include "SMPP_MessageFactory.hpp"

//#include "DummySMPPMessage.hpp"

namespace smpp_dmplx {

SMPP_MessageFactory::SMPP_MessageFactory()
  : _log(smsc::logger::Logger::getInstance("msg_fact"))
{}

void
SMPP_MessageFactory::registryCreatableMessage(SMPP_message* creatableMsg)
{
  smsc_log_debug(_log, "SMPP_MessageFactory::registryCreatableMessage::: registry prototype for message with code=[%08X]", creatableMsg->getCommandId());
  _cloenableMessages.insert(std::make_pair(creatableMsg->getCommandId(), creatableMsg));
}

std::auto_ptr<SMPP_message>
SMPP_MessageFactory::createSmppMessage(smpp_dmplx::BufferedInputStream& ioBuf)
{
  uint32_t msgCode = ioBuf.readUInt32();

  smsc_log_debug(_log, "SMPP_MessageFactory::createSmppMessage::: try create message for msgcode=[%08X]", msgCode);

  registry_t::iterator iter = _cloenableMessages.find(msgCode) ;
  if ( iter != _cloenableMessages.end() )
    return iter->second->clone(msgCode, ioBuf);

  if ( msgCode & 0x80000000)
    return _genericResponse.clone(msgCode, ioBuf);
  else
    return _genericRequest.clone(msgCode, ioBuf);
}

}
