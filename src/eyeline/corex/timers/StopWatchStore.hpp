/* ************************************************************************** *
 * MT Realtime timers watching facility: 
 * StopWatchStore - a store of timers FSM objects. 
 * ************************************************************************** */
#ifndef _CORE_TIMERS_STOPWATCH_STORE_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_STOPWATCH_STORE_HPP

#include <list>
#include <vector>

#include "eyeline/corex/timers/StopWatch.hpp"

namespace eyeline {
namespace corex {
namespace timers {


class StopWatchStore {
protected:
  mutable smsc::core::synchronization::Mutex  _sync;
  /* */
  typedef std::vector<StopWatchFSM *> SWArray;
  typedef std::list<uint32_t>         SWPool;

  SWArray   _store;
  SWPool    _pool;

  const TimeWatcherIdent &   _ownerId;

public:
  StopWatchStore(const TimeWatcherIdent & owner_id)
    : _ownerId(owner_id)
  { }
  ~StopWatchStore();

  //
  void reserve(uint32_t sw_num);
  //
  void getTimer(uint32_t sw_id, StopWatchGuard & p_lock) const;
  //
  void allcTimer(StopWatchGuard & p_lock);

  //Returns false if there is no such timer or there are
  //already too many references to that timer.
  bool refTimer(uint32_t sw_id);

  //Returns false if there is no such timer
  bool unrefTimer(uint32_t sw_id);
};

} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_STOPWATCH_STORE_HPP */

