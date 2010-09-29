/* ************************************************************************** *
 * Synchronization primitive(s): Guarded State primitive.
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_GUARDED_STATE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __CORE_SYNCHRONIZATION_GUARDED_STATE_HPP

#include "core/synchronization/Condition.hpp"

namespace smsc {
namespace core {
namespace synchronization {

// ********************************************************************
// Special 'State' primitive, that may be locked in order to postpone
// state switching until it will be unlocked.
// NOTE.1:  SwitchTo() method doesn't block on locked 'State' !
// NOTE.2:  if several SwitchTo() call is issued while state is locked,
//          the last requested value will be used in postponed state
//          switching.
// ********************************************************************
template <
    class _StateTArg //must have _StateTArg(0) defined
> class GuardedState_T {
protected:
  mutable Mutex _sync;
  Condition     _event;
  bool          _locked;
  _StateTArg    _curState;
  _StateTArg    _nextState;
  pthread_t     _thrId;

  GuardedState_T(const GuardedState_T &);
  void operator=(const GuardedState_T &);

public:
  static const _StateTArg _zeroState(void) { return  _StateTArg(0); }

  GuardedState_T()
    : _locked(false), _curState(_zeroState()), _nextState(_zeroState())
    , _thrId((pthread_t)(-1))
  { }

  GuardedState_T(_StateTArg use_state)
    : _locked(false), _curState(use_state), _nextState(_zeroState())
    , _thrId((pthread_t)(-1))
  { }
  ~GuardedState_T()
  { }

  _StateTArg get(void) const
  {
    MutexGuard grd(_sync);
    return _curState; 
  }

  bool isLocked(void) const
  {
    MutexGuard grd(_sync);
    return _locked;
  }

  _StateTArg Lock(void)
  {
    MutexGuard grd(_sync);
    if (_locked) //wait for Unlock() signal
      _event.WaitOn(_sync);
  
    _locked = true;
    _nextState = _curState;
    _thrId = pthread_self();
    return _curState;
  }

  bool TryLock(void)
  {
    MutexGuard grd(_sync);
    if (_locked)
      return false;

    _locked = true;
    _nextState = _curState;
    _thrId = pthread_self();
    return true;
  }

  void Unlock(void)
  {
    MutexGuard grd(_sync);
    if (_locked) {
      _curState = _nextState;
      _nextState = _zeroState();
      _thrId = (pthread_t)(-1);
      _locked = false;
      _event.Signal();  //awake one of awaiting Lock()
    }
  }

  void switchTo(_StateTArg use_state)
  {
    MutexGuard grd(_sync);
    if (_locked)
      _nextState = use_state;
    else
      _curState = use_state;
  }

  void reset(void) { switchTo(_zeroState()); }
};

}//namespace synchronization
}//namespace core
}//namespace smsc

#endif /* __CORE_SYNCHRONIZATION_GUARDED_STATE_HPP */

