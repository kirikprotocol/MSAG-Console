#ifndef __EYELINE_SMPPMULTIPLEXER_BALANCERS_SMSCLINKSET_HPP__
# define __EYELINE_SMPPMULTIPLEXER_BALANCERS_SMSCLINKSET_HPP__

# include <sys/types.h>
# include <string>

# include "core/synchronization/Mutex.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"
# include "eyeline/load_balancer/io_subsystem/Message.hpp"

namespace eyeline {
namespace load_balancer {
namespace balancers {

class SMSCLinkSet : public io_subsystem::LinkSet {
public:
  SMSCLinkSet(const io_subsystem::LinkId& link_set_id, unsigned total_number_of_links)
    : io_subsystem::LinkSet(link_set_id, total_number_of_links),
    _linkNumInSet(0) {}

  using io_subsystem::LinkSet::send;
  virtual io_subsystem::LinkId send(const io_subsystem::Message& message);
private:
  unsigned _linkNumInSet;
  smsc::core::synchronization::Mutex _sendLock;
};

}}}

#endif
