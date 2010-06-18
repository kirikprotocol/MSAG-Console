/* ************************************************************************* *
 * Lightweight Array element traits definition.
 * ************************************************************************* */
#ifndef __LWARRAY_TRAITS_DEFS
#ident "@(#)$Id$"
#define __LWARRAY_TRAITS_DEFS

#include <inttypes.h>

namespace eyeline {
namespace util {

template <
  class _TArg   //Element of array, must have a default & copying constructors!
>
struct LWArrayTraits {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(_TArg * use_dst, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_dst + i)_TArg();
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(_TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_dst + i)_TArg(use_src[i]);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void destroy(_TArg * use_buf, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      use_buf[i].~_TArg();
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void reset(_TArg * use_buf, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i) {
      use_buf[i].~_TArg();
      new (use_buf + i)_TArg();
    }
  }
  //Copies elements starting from first to last
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void copy(_TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      use_dst[i] = use_src[i];
  }
  //shifts number of elements to right
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  { //copy elements starting from last to first
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_buf + num_elem - 1 - i + shift_sz)_TArg(((const _TArg *)use_buf)[num_elem - 1 - i]);
  }
  //shifts number of elements to left
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  { //copy elements starting from first to last
    for (_SizeTypeArg i = 0; i < (num_elem - shift_sz); ++i)
      use_buf[i] = use_buf[i + shift_sz];
  }
};

} //util
} //eyeline

#endif /* __LWARRAY_TRAITS_DEFS */

