#include <utility>
#include "util/Exception.hpp"
#include "IdleLinksMonitor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
IdleLinksMonitor::insert(const LinkId& new_link_id, time_t keep_alive_epoche_time)
{
  link_timeouts_t::iterator insResIter =
    _link_timeouts.insert(std::make_pair(keep_alive_epoche_time,
                                         std::make_pair(new_link_id, I1(linkid_hash_t::iterator()))
                                         )
                          );

  std::pair<linkid_hash_t::iterator, bool> scndInsRes = _linkIdToTimeoutHash.insert(std::make_pair(new_link_id, I2(insResIter)));
  if ( !scndInsRes.second )
    throw smsc::util::Exception("IdleLinksMonitor::insert::: can't insert value to linkIdToTimeoutHash");
  insResIter->second.second.i1 = scndInsRes.first;
}

void
IdleLinksMonitor::remove(const LinkId& link_id)
{
  linkid_hash_t::iterator iter = _linkIdToTimeoutHash.find(link_id);
  if ( iter != _linkIdToTimeoutHash.end() ) {
    link_timeouts_t::iterator lnkTimeoutIter = iter->second.i2;
    _link_timeouts.erase(lnkTimeoutIter);
    _linkIdToTimeoutHash.erase(iter);
  }
}

bool
IdleLinksMonitor::fetchExpiredLink(LinkId* inactive_link_id)
{
  if ( _link_timeouts.empty() ) return false;

  link_timeouts_t::iterator iter_to_1st_elem = _link_timeouts.begin();
  if ( iter_to_1st_elem->first > time(0) )
    return false;

  *inactive_link_id = iter_to_1st_elem->second.first;
  _linkIdToTimeoutHash.erase(iter_to_1st_elem->second.second.i1);
  _link_timeouts.erase(iter_to_1st_elem);

  return true;
}

}}}
