#include "Publisher.hpp"

void
smpp_dmplx::Publisher::addSubscriber(SMPP_Subscriber* subscriber)
{
  _subscriberList.push_back(subscriber);
}

void
smpp_dmplx::Publisher::publish(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  for(SubscriberList_t::iterator iter = _subscriberList.begin(); iter != _subscriberList.end(); ++iter) {
    if ( (*iter)->handle(smpp, socket) == SMPP_Subscriber::RequestWasProcessed )
      break;
  }
}
