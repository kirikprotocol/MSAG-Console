#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/corex/timers/StopWatch.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::ReverseMutexGuard;

/* ************************************************************************** *
 * class StopWatchFSM implementation:
 * ************************************************************************** */
void StopWatchFSM::mkIdent(void)
{
  snprintf(_idStr.str + _idPos, _idStr.capacity() - _idPos - 1, ":%u{%u}", _id, _usage);
}


/* ************************************************************************** *
 * NOTE: all next FSM switching methods require StopWatchFSM being locked !!!
 * ************************************************************************** */
//switches FSM to swInited, is called only either while timer creation or
//while reusing already released timer
bool StopWatchFSM::init(TimerListenerIface * listener, const ExpirationTime & tgt_time,
                        const OpaqueArg * opaque_obj/* = NULL*/)
{
  if (_swState != swIdle)
    return false;
  _eventHdl = listener;
  if (opaque_obj)
    _opaqueObj = *opaque_obj;
  else
    _opaqueObj.clear();

  _swState = swInited;
  _nextState = swIsToSignal;
  ++_usage;
  _tgtTime = tgt_time;
  mkIdent();
  return true;
}
//switches FSM to swActive, returns false if FSM isn't in swInited state
bool StopWatchFSM::activate(void)
{
  if (_swState != swInited)
    return false;
  _swState = swActive;
  return true;
}
//switches FSM to swIsToSignal
bool StopWatchFSM::setToSignal(void)
{
  if (_swState != swActive)
    return false;
  _swState = swIsToSignal;
  return true;
}
//tries to switche FSM to swInited, returns true if succeeded,
//false if stopwatch is currently signaling
bool StopWatchFSM::stop(void)
{
  if (_swState == swSignalling) { //NOTE: refCount cann't be zero at this stage
    _nextState = swInited;
    return false;
  }
  _swState = swInited;
  _nextState = swIsToSignal;
  return true;
}

//tries to switche FSM to swIdle, returns true if succeeded,
//false if stopwatch is currently signaling
bool StopWatchFSM::release(void)
{
  if (_swState == swSignalling) {
    _nextState = swIdle;
    return false;
  }
  _swState = swIdle;
  _nextState = swIsToSignal;
  return true;
}

//Notifies StopWatch listener and switches FSM to eiher swInited or swIdle states.
TimeWatcherIface::SignalResult_e StopWatchFSM::notify(void)
{
  TimeWatcherIface::SignalResult_e rval = TimeWatcherIface::evtOk;
  if (_swState != swIsToSignal)       //either wrong state or signalling was cancelled
    return rval;

  _swState = swSignalling; //block FSM state transition until completion
  {
    ReverseMutexGuard grd(*this);
    rval = _eventHdl->onTimerEvent(_idStr, _opaqueObj.empty() ? NULL : &_opaqueObj);
  }
  if ((rval == TimeWatcherIface::evtOk) && (_nextState == swIsToSignal))
    _nextState = swInited; //no need to resignal

  //switch to _nextState (swIdle or swInited or swIsToSignal)
  _swState = _nextState;
  _nextState = swIsToSignal;
  return rval;
}


} //timers
} //corex
} //eyeline

