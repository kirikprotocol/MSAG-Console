#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "core/timers/TimeoutsQueue.hpp"

namespace smsc {
namespace core {
namespace timers {
/* ************************************************************************** *
 * class TimeoutsQueue implementation:
 * ************************************************************************** */
//Finds chain with lesser expiration time at front
void TimeoutsQueue::findNextChain(void)
{
  if (mTmsCnt) {
    //find 1st non-empty chain
    mNextChain = 0;
    while ((mNextChain < mTmsMap.size()) && mTmsMap.at(mNextChain).empty())
      ++mNextChain;

    if (mNextChain < mTmsMap.size()) {
      //scan next chains for one with lesser expiration time at front
      TMSChainsMap::size_type cPos = mNextChain;
      for (++cPos; cPos < mTmsMap.size(); ++cPos) {
        if (!mTmsMap.at(cPos).empty()
            && (mTmsMap.at(cPos).front().getTms() < mTmsMap.at(mNextChain).front().getTms()))
          mNextChain = cPos;
      }
    }
  } else
    mNextChain = mTmsMap.npos();
}

// -----------------------------------------------
// -- ExpirationQueueIface mehods implementation
// -----------------------------------------------
//Returns false in case of unsupported kind of timer expiration time,
//or queue overloading.
bool TimeoutsQueue::insert(TimerLink & sw_link)
{
  const ExpirationTime & tgtTime = sw_link->getTgtTime();
  if (!tgtTime.isValueTMO())
    return false;

  TMSChainsMap::size_type tPos = mTmsMap.find_crit(tgtTime.getValueTMO());
  if (tPos == mTmsMap.npos()) {
    tPos = mTmsMap.insert(TMSChain(tgtTime.getValueTMO()));
    if (tPos == mTmsMap.npos())
      return false;
  }
  TMSChain & pChain = mTmsMap.at(tPos);
  bool    checkNext = pChain.empty();

  pChain.push_back(sw_link);
  ++mTmsCnt;
  if (checkNext) { //update 'mNextChain'
    if ((mNextChain == mTmsMap.npos())
        || (pChain.front().getTms() < mTmsMap.at(mNextChain).front().getTms()))
      mNextChain = tPos;
  }
  return true;
}
//
void TimeoutsQueue::unlink(TimerLink & sw_link)
{
  if (!mTmsCnt)
    return;

  const ExpirationTime & tgtTime = sw_link->getTgtTime();
  TMSChainsMap::size_type tPos = mTmsMap.find_crit(tgtTime.getValueTMO());
  if ((tPos < mTmsMap.size()) && !mTmsMap.at(tPos).empty())  {
    TimerUId  frontId = mTmsMap.at(tPos).front()->getUIdx();
    mTmsMap.at(tPos).unlink(sw_link);
    --mTmsCnt;
    if ((frontId == sw_link->getUIdx()) && (mNextChain == tPos))
      findNextChain();
  }
}

//Exports nearest expiration time of all entries
//Returns false if queue is empty.
bool TimeoutsQueue::nextTime(struct timespec & exp_tm) const
{
  if (!mTmsCnt)
    return false;
  exp_tm = mTmsMap.at(mNextChain).front().getTms();
  return true;
}

//Exports the last expiration time of all entries
//Returns false if queue is empty.
bool TimeoutsQueue::lastTime(struct timespec & exp_tm) const
{
  if (!mTmsCnt)
    return false;

  exp_tm.tv_sec = exp_tm.tv_nsec = 0;
  for (TMSChainsMap::size_type cPos = 0; cPos < mTmsMap.size(); ++cPos) {
    if (!mTmsMap.at(cPos).empty() && (exp_tm < mTmsMap.at(cPos).back().getTms()))
      exp_tm = mTmsMap.at(cPos).back().getTms();
  }
  return true;
}

//Pops one of entry with expiration time less than specified.
bool TimeoutsQueue::popExpired(const struct timespec & exp_tm, TimerRef & sw_ref)
{
  if (!mTmsCnt || (exp_tm < mTmsMap.at(mNextChain).front().getTms()))
    return false;

  mTmsMap.at(mNextChain).pop_front(sw_ref);
  --mTmsCnt;
  findNextChain();
  return true;
}

//Pops one of entry with expiration time less than current one.
bool TimeoutsQueue::popExpired(TimerRef & sw_ref)
{
  struct timespec curTm;
  TimeSlice::getRealTime(curTm);
  return popExpired(curTm, sw_ref);
}

} //timers
} //core
} //smsc

