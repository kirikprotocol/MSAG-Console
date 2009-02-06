#include "Publisher.hpp"

namespace smpp_dmplx {

void
Publisher::addSubscriber(SMPP_Subscriber* subscriber)
{
  _subscriberList.push_back(subscriber);
}

void
Publisher::publish(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  for(SubscriberList_t::iterator iter = _subscriberList.begin(); iter != _subscriberList.end(); ++iter) {
    if ( (*iter)->handle(smpp, socket) == SMPP_Subscriber::RequestWasProcessed )
      break;
  }
}

}
