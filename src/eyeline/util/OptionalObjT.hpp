/* ************************************************************************* *
 * Helper template class for creation of arbitrary type object in 
 * preallocated memory.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_OBJECT_PRODUCER_HPP
#ident "@(#)$Id$"
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

  void cleanUp(void)
  {
    if (_ptr)
      _ptr->~_TArg();
  }

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
    cleanUp();
  }

  //
  _TArg  & init(void)
  {
    cleanUp();
    return *(_ptr = new (_mem._buf)_TArg());
  }
  //
  _TArg  * get(void) { return _ptr; }
  //
  const _TArg  * get(void) const { return _ptr; }
};


} //util
} //eyeline

#endif /* __EYELINE_UTIL_OBJECT_PRODUCER_HPP */

