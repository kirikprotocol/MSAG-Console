/* ************************************************************************* *
 * Lightweight Array element traits definition.
 * ************************************************************************* */
#ifndef __CORE_BUFFERS_LWARRAY_TRAITS_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_LWARRAY_TRAITS_DEFS

#include <memory.h>

namespace smsc {
namespace core {
namespace buffers {

// ------------------------------------------------------------------------
// Array traits for elements with complex object constructor/destructor.
// ------------------------------------------------------------------------
template <
  class _TArg   //Element of array, must have a default & copying constructors!
>
struct LWArrayTraits_T {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(_TArg * use_dst, const _TArg & use_val, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_dst + i)_TArg(use_val);
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
  //Resets elements starting from first to last
  //NOTE: all ('num_elem') elements of destination MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(_TArg * use_buf, _SizeTypeArg num_elem, const _TArg & use_val)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i) {
      use_buf[i].~_TArg();
      new (use_buf + i)_TArg(use_val);
    }
  }
  //Copies elements starting from first to last
  //NOTE: source and destination MUST NOT OVERLAP
  //NOTE: all ('num_elem') elements of destination MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(_TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i) {
      use_dst[i].~_TArg();
      new (use_dst + i)_TArg(use_src[i]);
    }
  }

  //Compares elements starting from first to last. Returns:
  // -1 if lesser than, 
  // 0  if equal to,
  // 1  if grater than
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static short compare(const _TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i) {
      if (use_dst[i] < use_src[i])
        return -1;
      if (use_src[i] < use_dst[i])
        return 1;
    }
    return 0;
  }

  //shifts number of elements to right.
  //NOTE: all elements in range ['num_elems', 'num_elem' + 'shift_sz']  MUST BE DESTROYED
  //      prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  { //copy elements starting from last to first
    _SizeTypeArg i = num_elem;
    while (i > 0) {
      --i;
      new (use_buf + i + shift_sz)_TArg(use_buf[i]);
    }
  }
  //shifts number of elements to left (cuts off first 'shift_sz' elements)
  //NOTE: all ('num_elem') elements MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  { //copy elements starting from first to last
    for (_SizeTypeArg i = 0; i < num_elem - shift_sz; ++i) {
      use_buf[i].~_TArg();
      new (use_buf + i)_TArg(use_buf[shift_sz + i]);
    }
  }
};


// -----------------------------------------------------------
// Array traits for elements that is POD objects
// -----------------------------------------------------------
template <
  class _TArg   //Element of array, must have a default & copying constructors!
>
struct LWArrayTraitsPOD_T {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(_TArg * use_dst, const _TArg & use_val, _SizeTypeArg num_elem)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_dst + i)_TArg(use_val);
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
#ifdef LW_ARRAY_DEBUG
    memset(use_buf, 0, sizeof(_TArg)*num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //Resets elements starting from first to last
  //NOTE: all ('num_elem') elements of destination MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(_TArg * use_buf, _SizeTypeArg num_elem, const _TArg & use_val)
  {
    for (_SizeTypeArg i = 0; i < num_elem; ++i)
      new (use_buf + i)_TArg(use_val);
  }
  //Copies elements starting from first to last
  //NOTE: source and destination MUST NOT OVERLAP
  //NOTE: all ('num_elem') elements of destination MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(_TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(_TArg)*num_elem);
  }

  //Compares elements starting from first to last. Returns:
  // -1 if lesser than, 
  // 0  if equal to,
  // 1  if grater than
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static short compare(const _TArg * use_dst, const _TArg * use_src, _SizeTypeArg num_elem)
  {
    return (short)memcmp(use_dst, use_src, sizeof(_TArg)*num_elem);
  }

  //shifts number of elements to right.
  //NOTE: all elements in range ['num_elems', 'num_elem' + 'shift_sz']  MUST BE DESTROYED
  //      prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    //copy elements starting from last to first
    memmove(use_buf + shift_sz, (const void *)use_buf, sizeof(_TArg)*num_elem);
  }
  //shifts number of elements to left (cuts off first 'shift_sz' elements)
  //NOTE: all ('num_elem') elements MUST BE CONSTRUCTED prior to this call!
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(_TArg * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    //copy elements starting from first to last
    memmove(use_buf, (const void *)(use_buf + shift_sz), sizeof(_TArg)*(num_elem - shift_sz));
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_LWARRAY_TRAITS_DEFS */

