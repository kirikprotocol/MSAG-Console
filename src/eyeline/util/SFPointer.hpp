/* ************************************************************************** *
 * Statefull pointer.
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_STATEFULL_POINTER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __CORE_SYNCHRONIZATION_STATEFULL_POINTER_HPP

#include "eyeline/util/GuardedState.hpp"

namespace smsc {
namespace core {
namespace synchronization {

// ---------------------------------------------------------------------
// Statefull pointer: may be changed immediately only in unlocked
// state, otherwise update is potponed until pointer will be unlocked.
// ---------------------------------------------------------------------
template < class _TArg >
class SFPointer_T  : public GuardedState_T<_TArg *> {
protected:
  SFPointer_T(const SFPointer_T &);
  void operator=(const SFPointer_T &);

public:
  SFPointer_T(_TArg * use_ptr = NULL)
    : GuardedState_T<_TArg *>(use_ptr)
  { }
  ~SFPointer_T()
  { }

  _TArg * operator->() const { return this->get(); }

  SFPointer_T & operator=(_TArg * use_ptr)
  {
    switchTo(use_ptr); return *this;
  }
};

// ---------------------------------------------------------------------
// Statefull pointer guard.
// ---------------------------------------------------------------------
template <class _TArg>
class SFPointerGuard_T {
private:
  SFPointer_T<_TArg> &  _lock;
  _TArg *               _ptr;

protected:
  SFPointerGuard_T(const SFPointerGuard_T &);
  void operator=(const SFPointerGuard_T &);

public:
  SFPointerGuard_T(SFPointer_T<_TArg> & grd_lock)
    : _lock(grd_lock), _ptr(0)
  {
    _ptr = _lock.Lock();
  }
  ~SFPointerGuard_T()
  {
    _lock.Unlock();
  }

  _TArg * get(void) const { return _ptr; }
  //
  _TArg * operator->() const { return _ptr; }
};


}//namespace synchronization
}//namespace core
}//namespace smsc

#endif /* __CORE_SYNCHRONIZATION_STATEFULL_POINTER_HPP */

