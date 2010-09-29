#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/corex/timers/TimeoutsQueue.hpp"

namespace eyeline {
namespace corex {
namespace timers {
/* ************************************************************************** *
 * class TimeoutsQueue implementation:
 * ************************************************************************** */
void TimeoutsQueue::findNextChain(void)
{
  if (!_tmsMap.empty()) {
    TMSChainsMap::const_iterator cit = _tmsMap.begin();
    _nextChain = cit;

    for (++cit; cit != _tmsMap.end(); ++cit) {
      if (!cit->second.empty()
          && (cit->second.front()._tgtTime) < _nextChain->second.front()._tgtTime)
        _nextChain = cit;
    }
  } else
    _nextChain = _tmsMap.end();
}

// -----------------------------------------------
// -- ExpirationQueueIface mehods implementation
// -----------------------------------------------
//
bool TimeoutsQueue::insert(uint32_t sw_id, const ExpirationTime & tgt_time)
{
  if (tgt_time.getKind() != ExpirationTime::valTmo)
    return false;

  TMSChainsMap::iterator it = _tmsMap.find(tgt_time.getValueTMO());
  if (it == _tmsMap.end()) {
    std::pair<TMSChainsMap::iterator, bool> res = 
      _tmsMap.insert(std::make_pair(tgt_time.getValueTMO(), Chain()));
    it = res.first;
  }
  Chain & tmoChain = it->second;
  bool    checkNext = !tmoChain.empty();

  tmoChain.insert(sw_id, tgt_time.adjust2TMS());
  ++_tmsCnt;
  if (checkNext)
    updateNextChain(it);

  return true;
}
//
void TimeoutsQueue::erase(uint32_t sw_id, const ExpirationTime & tgt_time)
{
  if (tgt_time.getKind() != ExpirationTime::valTmo)
    return;

  TMSChainsMap::iterator it = _tmsMap.find(tgt_time.getValueTMO());
  if (it != _tmsMap.end()) {
    uint32_t  frontId = it->second.front()._swId;
    it->second.erase(sw_id);
    --_tmsCnt;
    if ((frontId == sw_id) && (_nextChain == it))
      findNextChain();
  }
}


//Exports the last expiration time of all entries
//Returns false if queue is empty.
bool TimeoutsQueue::lastTime(struct timespec & exp_tm) const
{
  if (_tmsMap.empty())
    return false;

  exp_tm.tv_sec = exp_tm.tv_nsec = 0;

  for (TMSChainsMap::const_iterator cit = _tmsMap.begin();
                                    cit != _tmsMap.end(); ++cit) {
    if (!cit->second.empty()
        && (exp_tm < _nextChain->second.back()._tgtTime))
      exp_tm = _nextChain->second.back()._tgtTime;
  }
  return true;
}


//Pops one of entry with expiration time less than specified.
bool TimeoutsQueue::popExpired(const struct timespec & exp_tm, uint32_t & sw_id)
{
  if (empty() || (exp_tm < _nextChain->second.front()._tgtTime))
    return false;
  
  for (TMSChainsMap::iterator it = _tmsMap.begin(); it != _tmsMap.end(); ++it) {
    if (it->second.front()._tgtTime < exp_tm) {
      sw_id = it->second.front()._swId;
      it->second.pop_front();
      --_tmsCnt;
      if (it->second.empty() && (_nextChain == it))
        findNextChain();
      return true;
    }
  }
  return false;
}

//Pops one of entry with expiration time less than current one.
bool TimeoutsQueue::popExpired(uint32_t & sw_id)
{
  struct timespec curTm;
  TimeSlice::getRealTime(curTm);
  return popExpired(curTm, sw_id);
}


} //timers
} //corex
} //eyeline

