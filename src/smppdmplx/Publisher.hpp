#ifndef __SMPPDMPLX_PUBLISHER_HPP__
# define __SMPPDMPLX_PUBLISHER_HPP__

# include <list>
# include <memory>
# include <util/Singleton.hpp>

# include <smppdmplx/core_ax/network/Socket.hpp>
# include <smppdmplx/SMPP_message.hpp>
# include <smppdmplx/SMPP_Subscriber.hpp>

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
