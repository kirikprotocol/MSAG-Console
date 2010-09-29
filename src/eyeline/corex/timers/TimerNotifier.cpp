#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/corex/timers/TimerNotifier.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::MutexGuard;

const TimeSlice  _dflt_Shutdown_Tmo(200, TimeSlice::tuMSecs);

/* ************************************************************************** *
 * class TimerNotifier::SWSignalingTask implementation:
 * ************************************************************************** */
// -------------------------------------------
// -- ThreadedTask interface methods
// -------------------------------------------
const char * TimerNotifier::SWSignalingTask::taskName(void)
{
  if (_idStr.empty() && _swStore) {
    StopWatchGuard pSw;
    _swStore->getTimer(_swId, pSw);
    _idStr = pSw.get() ? pSw->IdStr() : "undefined";
  }
  return _idStr.c_str();
}

int TimerNotifier::SWSignalingTask::Execute(void)
{
  TimerSignalResult_e rval = TimeWatcherIface::evtOk;
  taskName();
  do {
    smsc_log_debug(_logger, "Timer[%s]: signalling ..", _idStr.c_str());
    {
      StopWatchGuard pSw;
      _swStore->getTimer(_swId, pSw);
      rval = pSw->notify();
    }
    if ((rval == TimeWatcherIface::evtResignal)) {
      smsc_log_debug(_logger, "Timer[%s]: is to resignal ..", _idStr.c_str());
      struct timespec tv = {0, 1000000}; //1 ms
      nanosleep(&tv, 0);
    }
  } while ((rval == TimeWatcherIface::evtResignal) && !isStopping);
  /**/
  _swStore->unrefTimer(_swId);
  _swStore = 0;
  return (int)rval;
}

void TimerNotifier::SWSignalingTask::onRelease(void)
{
  if (_swStore) { //task was inited but not executed!!!
    _swStore->unrefTimer(_swId);
    _swStore = 0;
  }
  _idStr.clear();
  isReleased = true;
  _owner->rlseTask(this);
}

/* ************************************************************************** *
 * class TimerNotifier::SWSTaskStore implementation:
 * ************************************************************************** */
void TimerNotifier::SWSTaskStore::reserveTasks(unsigned num_tasks)
{
  MutexGuard  grd(_sync);
  if (num_tasks > _store.capacity())
    _store.reserve(num_tasks);
  if (num_tasks > _store.size()) {
    memset(&_store[_store.size()], 0, (num_tasks - _store.size())*sizeof(void*));
    for (TasksArray::size_type i = _store.size(); i < num_tasks; ++i)
      _store[i] = new SWSignalingTask(*this, _logger);
  }
}
//
TimerNotifier::SWSignalingTask *
  TimerNotifier::SWSTaskStore::allcTask(uint32_t sw_id)
{
  MutexGuard  grd(_sync);
  SWSignalingTask * pTask;
  if (_pool.empty()) {
    _store.reserve(_store.size() + 1);
    _store[_store.size()] = pTask = new SWSignalingTask(*this, _logger);
  } else {
    pTask = _pool.front();
    _pool.pop_front();
  }
  pTask->Init(_swStore, sw_id);
  return pTask;
}

void TimerNotifier::SWSTaskStore::rlseTask(SWSignalingTask * p_task)
{
  MutexGuard  grd(_sync);
  _pool.push_back(p_task);
}

void TimerNotifier::SWSTaskStore::rlseAlltasks(void)
{
  MutexGuard  grd(_sync);
  for (TasksArray::iterator it = _store.begin(); it != _store.end(); ++it) {
    if (*it) {
      delete *it;
      *it = 0;
    }
  }
  _pool.clear();
}


} //timers
} //corex
} //eyeline

