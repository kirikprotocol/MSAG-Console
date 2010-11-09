/* ************************************************************************* *
 * Helper template class for creation of arbitrary type object in 
 * preallocated memory.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_OBJECT_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_UTIL_OBJECT_PRODUCER_HPP

#include <inttypes.h>

namespace eyeline {
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
  _TArg  & init(void)
  {
    clear();
    return *(_ptr = new (_mem._buf)_TArg());
  }
  //
  _TArg  * get(void) { return _ptr; }
  //
  const _TArg  * get(void) const { return _ptr; }
  //
  void clear(void)
  {
    if (_ptr)
      _ptr->~_TArg();
  }
};


} //util
} //eyeline

#endif /* __EYELINE_UTIL_OBJECT_PRODUCER_HPP */

