#include <utility>
#include "util/Exception.hpp"
#include "IdleLinksMonitor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
IdleLinksMonitor::insert(const LinkId& newLinkId, time_t keepAliveEpocheTime)
{
  link_timeouts_t::iterator ins_res_iter =
    _link_timeouts.insert(std::make_pair(keepAliveEpocheTime,
                                         std::make_pair(newLinkId, I1(linkid_hash_t::iterator()))
                                         )
                          );

  std::pair<linkid_hash_t::iterator, bool> scnd_ins_res = _linkIdToTimeoutHash.insert(std::make_pair(newLinkId, I2(ins_res_iter)));
  if ( !scnd_ins_res.second )
    throw smsc::util::Exception("IdleLinksMonitor::insert::: can't insert value to linkIdToTimeoutHash");
  ins_res_iter->second.second.i1 = scnd_ins_res.first;
}

void
IdleLinksMonitor::remove(const LinkId& linkId)
{
  linkid_hash_t::iterator iter = _linkIdToTimeoutHash.find(linkId);
  if ( iter != _linkIdToTimeoutHash.end() ) {
    link_timeouts_t::iterator lnk_timeout_iter = iter->second.i2;
    _link_timeouts.erase(lnk_timeout_iter);
    _linkIdToTimeoutHash.erase(iter);
  }
}

bool
IdleLinksMonitor::fetchExpiredLink(LinkId* inactiveLinkId)
{
  if ( _link_timeouts.empty() ) return false;

  link_timeouts_t::iterator iter_to_1st_elem = _link_timeouts.begin();
  if ( iter_to_1st_elem->first > time(0) )
    return false;

  *inactiveLinkId = iter_to_1st_elem->second.first;
  _linkIdToTimeoutHash.erase(iter_to_1st_elem->second.second.i1);
  _link_timeouts.erase(iter_to_1st_elem);

  return true;
}

}}}
