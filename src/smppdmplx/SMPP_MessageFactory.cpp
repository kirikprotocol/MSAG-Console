#include "SMPP_MessageFactory.hpp"
#include "DummySMPPMessage.hpp"

void
smpp_dmplx::SMPP_MessageFactory::registryCreatableMessage(SMPP_message* creatableMsg)
{
  _creatableMessageList.push_back(creatableMsg);
}

std::auto_ptr<smpp_dmplx::SMPP_message>
smpp_dmplx::SMPP_MessageFactory::createSmppMessage(smpp_dmplx::BufferedInputStream& ioBuf)
{
  uint32_t msgCode = ioBuf.readUInt32();
  for (std::list<SMPP_message*>::const_iterator iter=_creatableMessageList.begin(); iter != _creatableMessageList.end(); ++iter) {
    if ( (*iter)->checkMessageCodeEquality(msgCode) ) 
      return (*iter)->clone(msgCode, ioBuf);
  }
  return std::auto_ptr<SMPP_message>(new DummySMPPMessage());
}
