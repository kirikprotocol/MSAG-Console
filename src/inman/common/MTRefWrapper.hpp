/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __MT_REFWRAPPER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __MT_REFWRAPPER_HPP__

#include <pthread.h>

namespace smsc {
namespace core {
namespace synchronization {

// ********************************************************************
// 
// NOTE: Requires an external sync object.
// ********************************************************************
template <class _TArg>
class MTRefWrapper_T {
protected:
  volatile bool _locked;
  pthread_t     _thrId;
  _TArg *       _curState;
  _TArg *       _nextState;

  MTRefWrapper_T(const MTRefWrapper_T & cp_obj);

public:
  explicit MTRefWrapper_T(_TArg * use_ref = NULL)
    : _locked(false), _thrId((pthread_t)(-1))
    , _curState(use_ref), _nextState(0)
  { }
  ~MTRefWrapper_T()
  { }

  void Reset(_TArg * use_ref)
  {
    _locked = false;
    _thrId = (pthread_t)(-1);
    _curState = use_ref;  _nextState = 0;
  }

  _TArg * get(void) const { return _curState; }
  _TArg * operator->() const { return get(); }

  _TArg * getNext(void) const { return _locked ? _nextState : _curState; }

  _TArg * Lock(void)
  {
    _locked = true;
    if (_curState)
      _thrId = pthread_self();
    return (_nextState = _curState);
  }

  _TArg * UnLock(void)
  {
    _curState = _nextState;
    _nextState = 0;
    _thrId = (pthread_t)(-1);
    _locked = false;
    return _curState;
  }

  bool Unref(void)
  {
    if (!_locked) {
      _curState = _nextState = 0;
      return true;
    }
    if (_thrId == pthread_self()) {  //locked by same thread,
      _nextState = 0;                 //next Unlock() will perform Unref()
      return true;
    }
    return false;
  }
};


}//namespace synchronization
}//namespace core
}//namespace smsc

#endif /* __MT_REFWRAPPER_HPP__ */

