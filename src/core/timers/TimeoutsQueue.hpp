/* ************************************************************************** *
 * MT Realtime timers monitoring facility.
 * TimeoutsQueue: manages the expiration time in relative form.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMEOUTS_QUEUE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMEOUTS_QUEUE_HPP

#include "core/buffers/LWArraySortedT.hpp"
#include "core/timers/ExpirationQueueDefs.hpp"

namespace smsc {
namespace core {
namespace timers {

class TimeoutsQueue : public ExpirationQueueIface {
public:
  TimeoutsQueue() : mTmsCnt(0)
  {
    mNextChain = mTmsMap.npos();
  }
  virtual ~TimeoutsQueue()
  { }

  //Reserves resources for handling of 'num_tmo' timeout values.
  void reserve(uint16_t num_tmo) { mTmsMap.reserve(num_tmo); }

  // -----------------------------------------------
  // -- ExpirationQueueIface mehods implementation
  // -----------------------------------------------
  //NOTE: ExpirationTime is accepted only in valTmo form
  virtual bool empty(void) const { return mTmsCnt == 0; }
  //
  virtual TimerUId size(void) const { return mTmsCnt; }
  //Returns false if in case of unsupported kind of timer expiration time.
  virtual bool insert(TimerLink & sw_link);
  //NOTE: It's a caller responsibility to ensure that given TimerLink was
  //      inserted previously !!!
  virtual void unlink(TimerLink & sw_link);

  //Exports nearest expiration time of all entries
  //Returns false if queue is empty.
  virtual bool nextTime(struct timespec & exp_tm) const;
  //Exports the last expiration time of all entries
  //Returns false if queue is empty.
  virtual bool lastTime(struct timespec & exp_tm) const;

  //Pops one of entry with expiration time less than specified.
  //Returns true if succeeded.
  virtual bool popExpired(const struct timespec & exp_tm, TimerRef & sw_ref);
  //Pops one of entry with expiration time less than current one.
  //Returns true if succeeded.
  virtual bool popExpired(TimerRef & sw_ref);

protected:
  //Queue of expiration times based on same timeout value.
  //Arranged as intrusive FIFO list
  class TMSChain : public smsc::util::ObjWithCriterion_T<TimeSlice> {
  private:
    using smsc::util::ObjWithCriterion_T<TimeSlice>::setCriterion;

  protected:
    typedef smsc::core::buffers::IntrList<TimerLink> TMSQueue;

    TMSQueue    mQueue;
    
  public:
    TMSChain() : smsc::util::ObjWithCriterion_T<TimeSlice>()
    { }
    explicit TMSChain(const TimeSlice & use_crit)
      : smsc::util::ObjWithCriterion_T<TimeSlice>(use_crit)
    { }
    ~TMSChain()
    { }

    bool empty(void) const { return mQueue.empty(); }

    const TimerLink & back(void) const { return *mQueue.back(); }
    const TimerLink & front(void) const { return *mQueue.front(); }
    //
    void  push_back(TimerLink & sw_link) { mQueue.push_back(&sw_link); }
    void  unlink(TimerLink & sw_link)  { mQueue.erase(&sw_link); }
    void  pop_front(TimerRef & sw_ref)
    {
      sw_ref = front().getRef();
      mQueue.pop_front();
    }
  };

  typedef smsc::core::buffers::LWArraySorted_T<TMSChain, uint16_t, 4> TMSChainsMap;

  TimerUId                mTmsCnt;  //overall number of timespecs in queue
  TMSChainsMap            mTmsMap;
  TMSChainsMap::size_type mNextChain; //chain with nearest expiration time at front

  //Finds chain with lesser expiration time at front
  void findNextChain(void);
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMEOUTS_QUEUE_HPP */

