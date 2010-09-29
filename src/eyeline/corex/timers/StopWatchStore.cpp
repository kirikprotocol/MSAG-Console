#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/corex/timers/StopWatchStore.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************** *
 * class StopWatchStore implementation:
 * ************************************************************************** */
StopWatchStore::~StopWatchStore()
{
  for (SWArray::size_type i = 0; i < _store.size(); ++i) {
    if (_store[i]) {
      delete _store[i];
      _store[i] = 0;
    }
  }
  //_pool.clear();
}

//
void StopWatchStore::reserve(uint32_t sw_num)
{
  MutexGuard  grd(_sync);
  if (sw_num > _store.capacity())
    _store.reserve(sw_num);
  if (sw_num > _store.size()) {
    memset(&_store[_store.size()], 0, (sw_num - _store.size())*sizeof(void*));
    for (uint32_t i = (uint32_t)_store.size(); i < sw_num; ++i)
      _store[i] = new StopWatchFSM(i, _ownerId);
  }
}
//
void StopWatchStore::getTimer(uint32_t sw_id, StopWatchGuard & p_lock) const
{
  StopWatchFSM * pFsm;
  {
    MutexGuard  grd(_sync);
    pFsm = (sw_id < _store.size()) ? _store[sw_id] : 0;
  }
  p_lock.setPtr(pFsm);
}
//
void StopWatchStore::allcTimer(StopWatchGuard & p_lock)
{
  StopWatchFSM * pFsm;
  {
    MutexGuard  grd(_sync);
    if (!_pool.empty()) {
      pFsm = _store[_pool.front()];
      _pool.pop_front(); 
    } else {
      uint32_t swId = (uint32_t)_store.size();
      _store.reserve(swId + 1);
      _store[swId] = pFsm = new StopWatchFSM(swId, _ownerId);
    }
  }
  p_lock.setPtr(pFsm);
}

//Returns false if there is no such timer or there are
//already too many references to that timer.
bool StopWatchStore::refTimer(uint32_t sw_id)
{
  StopWatchGuard pFsm;
  getTimer(sw_id, pFsm);
  return (pFsm.get() && pFsm->Ref());
}

//Returns false if there are references remain
bool StopWatchStore::unrefTimer(uint32_t sw_id)
{
  StopWatchGuard pFsm;
  getTimer(sw_id, pFsm);

  if (pFsm.get()) {
    if (pFsm->UnRef() && pFsm->release()) {
      MutexGuard  grd(_sync);
      _pool.push_back(sw_id);
    }
    return true;
  }
  return false;
}

} //timers
} //corex
} //eyeline

