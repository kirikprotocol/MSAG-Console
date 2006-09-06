#ifndef __SMPPDMPLX_PUBLISHER_HPP__
# define __SMPPDMPLX_PUBLISHER_HPP__ 1

# include <core_ax/network/Socket.hpp>
# include "SMPP_message.hpp"
# include "SMPP_Subscriber.hpp"

# include <util/Singleton.hpp>
# include <list>
# include <memory>

namespace smpp_dmplx {

class Publisher : public smsc::util::Singleton<Publisher> {
public:
  void addSubscriber(SMPP_Subscriber* subscriber);
  void publish(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket);
private:
  typedef std::list<SMPP_Subscriber*> SubscriberList_t;

  SubscriberList_t _subscriberList;
};

}

#endif
