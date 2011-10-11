#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>

#include "core/timers/TimerFSM.hpp"

namespace smsc {
namespace core {
namespace timers {
/* ************************************************************************** *
 * class TimerFSM implementation:
 * ************************************************************************** */
const char * TimerFSM::nmState(ProcState_e st_val)
{
  switch (st_val) {
  case swIdle:        return "swIdle";
  case swInited:      return "swInited";
  case swActive:      return "swActive";
  case swIsToSignal:  return "swIsToSignal";
  case swSignalling:  return "swSignalling";
  default:;
  }
  return "unknown";
}

void TimerFSM::mkIdent(const char * id_pfx)
{
  strncpy(mStat.str, id_pfx, k_maxStatPfxSZ);
  mStat.str[k_maxStatPfxSZ] = 0;
  size_t pfxLen = mStat.length();
  if (pfxLen) {
    mStat += ".";
    ++pfxLen;
  }
  snprintf(mStat.str + pfxLen, mStat.capacity() - 1 - pfxLen, "%u:%u", this->getUIdx(), mUsage);
  mStat.str[mStat.capacity() - 1] = 0;
}

/* ************************************************************************** *
 * NOTE: all next FSM switching methods require TimerFSM being locked !!!
 * ************************************************************************** */
//Switches FSM to swInited.
//Returns false in case of inconsistent FSM state.
bool TimerFSM::init(TimerRefereeIface & use_rfr, const char * id_pfx,
                    TimerListenerIface & p_hdl, const ExpirationTime & tgt_time)
{
  if ((mSwState == swIdle) || ((mSwState == swInited) && mTgtTime.isValueTMO())) {
    mReferee = &use_rfr;
    mEventHdl = &p_hdl;
    mSwState = swInited;
    mNxtState = swIsToSignal;
    ++mUsage;
    mTgtTime = tgt_time;
    mkIdent(id_pfx);
    mLink.clear();
    return true;
  }
  return false;
}

//switches FSM to swActive and asks referee to enqueue itself
//returns:
// +1 - if succeeded. 
//  0 - if FSM isn't in swInited state.
// -1 - if expiration time is invalid or has a unsupported from.
// -2 - if referee unable to monitor timer due to its own state.
TimerFSM::ActivationResult_e
  TimerFSM::activate(const TimerFSM::ObjRef_t & this_ref)
{
  if (mSwState != swInited)
    return actFsmState;

  //check target time 
  struct timespec curTms;
  TimeSlice::getRealTime(curTms);
  struct timespec tgt_time = mTgtTime.adjust2TMS(&curTms);
  if (!(curTms < tgt_time))
    return actBadTime;
  
  mLink.init(this_ref, tgt_time);
  mSwState = swActive;
  ActivationResult_e res = mReferee->enqueue(mLink);
  if (res != actOk) {
    mSwState = swInited;
    mLink.clear();
  }
  return res;
}

//switches FSM to swIsToSignal
bool TimerFSM::setToSignal(void)
{
  if (mSwState != swActive)
    return false;
  mSwState = swIsToSignal;
  return true;
}

//tries to switche FSM to swInited, returns true if succeeded,
//false if stopwatch is currently signaling
bool TimerFSM::stop(void)
{
  if (mSwState == swSignalling) { //NOTE: refCount cann't be zero at this stage
    mNxtState = swInited;
    return false;
  }
  mSwState = swInited;
  mNxtState = swIsToSignal;
  if (mLink.isLinked()) {
    mReferee->unlink(mLink);
    mLink.clear();
  }
  return true;
}

//tries to switche FSM to swIdle, returns true if succeeded,
//false if stopwatch is currently signaling
bool TimerFSM::release(void)
{
  if (mSwState == swSignalling) {
    mNxtState = swIdle;
    return false;
  }
  mSwState = swIdle;
  mNxtState = swIsToSignal;
  if (mLink.isLinked()) {
    mReferee->unlink(mLink);
    mLink.clear();
    mReferee = 0;
  }
  return true;
}

//Notifies StopWatch listener and switches FSM to eiher swInited or swIdle states.
TimerEventResult_e TimerFSM::notify(void)
{
  TimerEventResult_e rval = TimerListenerIface::evtOk;
  if (mSwState != swIsToSignal)       //either wrong state or signalling was cancelled
    return rval;

  mSwState = swSignalling; //block FSM state transition until completion
  {
    smsc::core::synchronization::ReverseMutexGuard rGrd(*this);
    rval = mEventHdl->onTimerEvent(this->getUIdx(), mStat.c_str());
  }
  if ((rval == TimerListenerIface::evtOk) && (mNxtState == swIsToSignal)) {
    mNxtState = mTgtTime.isValueTMO() ? swInited : swIdle;
  }
  //switch to next state: swIdle, swInited or swIsToSignal
  mSwState = mNxtState;
  mNxtState = swIsToSignal;

  if (mSwState != swIsToSignal) { //no need to resignal
    mLink.clear();
    if (!mTgtTime.isValueTMO())
      mReferee = 0;
  }
  return rval;
}

} //timers
} //core
} //smsc

