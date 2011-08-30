/* ************************************************************************* *
 * Helper template class for creation of arbitrary type object in 
 * preallocated memory.
 * ************************************************************************* */
#ifndef __SMSC_UTIL_OBJECT_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_OBJECT_PRODUCER_HPP

#include <inttypes.h>

namespace smsc {
namespace util {

template <
  class _TArg  //must have default & copying constructor
> 
class OptionalObj_T {
protected:
  union {
    void *    _aligner;
    uint8_t   _buf[sizeof(_TArg)];
  } _mem;

  _TArg  * _ptr;

public:
  OptionalObj_T() : _ptr(0)
  {
    _mem._aligner = 0;
  }
  OptionalObj_T(const OptionalObj_T & use_obj) : _ptr(0)
  {
    _mem._aligner = 0;
    if (use_obj.get())
      _ptr = new (_mem._buf)_TArg(*use_obj.get());
  }
  //
  ~OptionalObj_T()
  {
    clear();
  }

  //
  void clear(void)
  {
    if (_ptr) {
      _ptr->~_TArg();
      _mem._aligner = _ptr = 0;
    }
  }

  //
  _TArg  & init(void)
  {
    clear();
    return *(_ptr = new (_mem._buf)_TArg());
  }
  //
  _TArg  & init(const _TArg & use_obj)
  {
    clear();
    return *(_ptr = new (_mem._buf)_TArg(use_obj));
  }

  //
  _TArg  * get(void) { return _ptr; }
  //
  const _TArg  * get(void) const { return _ptr; }
  //
  _TArg * operator->() { return _ptr; }
  const _TArg * operator->() const { return _ptr; }

  //
  OptionalObj_T & operator=(const OptionalObj_T & cp_obj)
  {
    if (this != &cp_obj) {
      clear();
      if (cp_obj._ptr)
        _ptr = new (_mem._buf)_TArg(*cp_obj._ptr);
    }
    return *this;
  }
};


} //util
} //smsc

#endif /* __SMSC_UTIL_OBJECT_PRODUCER_HPP */

