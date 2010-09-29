/* ************************************************************************** *
 * MT Realtime timers watching facility: 
 * StopWatchFSM  - an object that represents timer lifecycle.
 * ************************************************************************** */
#ifndef _CORE_TIMERS_STOPWATCH_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_STOPWATCH_HPP

#include "core/synchronization/Mutex.hpp"
#include "eyeline/corex/timers/TMWatcherDefs.hpp"
#include "eyeline/corex/timers/ExpirationTime.hpp"

namespace eyeline {
namespace corex {
namespace timers {

//StopWatch Finite State Machine
class StopWatchFSM : public smsc::core::synchronization::Mutex {
public:
  enum SWState_e {
      swIdle  = 0   //StopWatch is idle (unused)
    , swInited      //StopWatch is initialized (but not activated)
    , swActive      //StopWatch expiration is currently monitored
    , swIsToSignal  //StopWatch is awaiting to be signalled.
    , swSignalling  //StopWatch is currently signalling
  };

  StopWatchFSM(uint32_t tm_id, const TimeWatcherIdent & owner_id)
    : _id(tm_id), _idStr(owner_id), _idPos((unsigned)owner_id.length()), _usage(0)
    , _refNum(0), _swState(swIdle), _nextState(swIsToSignal), _eventHdl(0)
  {
    mkIdent();
  }
  ~StopWatchFSM()
  { }

  uint32_t Id(void) const { return _id; }

  // -- ******************************************************************** --
  // -- NOTE: following methods require StopWatchFSM being locked !!!
  // -- ******************************************************************** --
  //
  SWState_e State(void) const { return _swState; }
  //
  const char * IdStr(void) const { return _idStr.c_str(); }
  //
  const ExpirationTime & tgtTime(void) const { return _tgtTime; }

  bool hasRef(void) const { return _refNum != 0; }

  //Returns false if too much references already present
  bool Ref(void)
  {
    if (!++_refNum) { //too much refs !!!
      --_refNum; return false;
    }
    return true;
  }
  //Returns true if no references remain
  bool UnRef(void)
  {
    if (_refNum)
      --_refNum;
    return !_refNum;
  }
  //switches FSM to swInited
  bool init(TimerListenerIface * listener, const ExpirationTime & tgt_time,
            const OpaqueArg * opaque_obj = NULL);
  //switches FSM to swActive, returns false if FSM isn't in swInited state
  bool activate(void);
  //switches FSM to swIsToSignal 
  bool setToSignal(void);
  //switches FSM to swSignalling, notifies StopWatch listener and
  //switches FSM to eiher swInited or swIdle states.
  TimerSignalResult_e notify(void);
  //tries to switche FSM to swInited, returns true if succeeded,
  //false if StopWatch is currently signaling
  bool stop(void);
  //tries to switche FSM to swIdle, returns true if succeeded,
  //false if StopWatch is currently signaling
  bool release(void);

private:
  const uint32_t      _id;
  TimerIdent          _idStr;
  unsigned            _idPos;
  uint32_t            _usage;
  uint32_t            _refNum;    //number of active timer handles referencing this StopWatch
  SWState_e           _swState;   //current StopWatch FSM state
  SWState_e           _nextState; //target FSM state the StopWatch ought to advance after signalling
  ExpirationTime      _tgtTime;
  TimerListenerIface* _eventHdl;
  OpaqueArg           _opaqueObj; //externally defined object that is passed to listener

  void mkIdent(void);
};


class StopWatchGuard {
private:
  StopWatchFSM * _swFsm;

public:
  StopWatchGuard(StopWatchFSM * use_fsm = NULL) : _swFsm(use_fsm)
  {
    if (_swFsm)
      _swFsm->Lock();
  }
  //
  ~StopWatchGuard()
  {
    if (_swFsm)
      _swFsm->Unlock();
  }

  //takes ownership of StopWatch
  void setPtr(StopWatchFSM * use_fsm)
  {
    if (_swFsm)
      _swFsm->Unlock();
    if ((_swFsm = use_fsm) != NULL)
      _swFsm->Lock();
  }
  //takes ownership of StopWatch
  void setPtr(StopWatchGuard & use_ptr)
  {
    if (_swFsm)
      _swFsm->Unlock();
    if ((_swFsm = use_ptr._swFsm) != NULL) {
      use_ptr._swFsm->Unlock();
      use_ptr._swFsm = 0;
      _swFsm->Lock();
    }
  }

  //
  StopWatchFSM * get(void) const { return _swFsm; }
  //
  StopWatchFSM * operator->() const { return _swFsm; }
};


} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_STOPWATCH_HPP */

