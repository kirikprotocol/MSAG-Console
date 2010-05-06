#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IDLELINKSMONITOR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IDLELINKSMONITOR_HPP__

# include <sys/types.h>
# include <map>
# include <utility>

# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IdleLinksMonitor {
public:
  void insert(const LinkId& new_link_id, time_t keep_alive_epoche_time);
  void remove(const LinkId& link_id);
  bool fetchExpiredLink(LinkId* inactive_link_id);
private:
  struct I1;
  struct I2;

  typedef std::pair<LinkId, I1> map_value_t;

  typedef std::multimap<time_t, map_value_t> link_timeouts_t;
  link_timeouts_t _link_timeouts;

  typedef std::map<LinkId, I2> linkid_hash_t;
  linkid_hash_t _linkIdToTimeoutHash;

  struct I1 {
    I1(linkid_hash_t::iterator i)
      : i1(i) {}
    linkid_hash_t::iterator i1;
  };

  struct I2 {
    I2(link_timeouts_t::iterator i)
      : i2(i) {}
    link_timeouts_t::iterator i2;
  };
};

}}}

#endif
