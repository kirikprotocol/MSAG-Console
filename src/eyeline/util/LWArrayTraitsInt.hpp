/* ************************************************************************* *
 * Lightweight Array element traits: partial specializations for integer types.
 * ************************************************************************* */
#ifndef __LWARRAY_TRAITS_INT_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __LWARRAY_TRAITS_INT_DEFS

#include <memory.h>

#include "eyeline/util/LWArrayTraits.hpp"

namespace eyeline {
namespace util {


template </* _TArg = uint8_t */>
struct LWArrayTraits<uint8_t> {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint8_t * use_dst, _SizeTypeArg num_elem)
  {
#ifdef LW_ARRAY_DEBUG
    memset(use_dst, 0, num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint8_t * use_dst, const uint8_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void destroy(uint8_t * use_buf, _SizeTypeArg num_elem)
  {
#ifdef LW_ARRAY_DEBUG
    memset(use_buf, 0, num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void reset(uint8_t * use_buf, _SizeTypeArg num_elem)
  {
    memset(use_buf, 0, num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void copy(uint8_t * use_dst, const uint8_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, num_elem);
  }
  //shifts number of elements to right
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(uint8_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf + shift_sz, (const uint8_t *)use_buf, num_elem);
  }
  //shifts number of elements to left
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(uint8_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf - shift_sz, (const uint8_t *)use_buf, num_elem);
  }
};


template </* _TArg = uint16_t */>
struct LWArrayTraits<uint16_t> {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint16_t * use_dst, _SizeTypeArg num_elem)
  {
#ifdef LW_ARRAY_DEBUG
    memset(use_dst, 0, sizeof(uint16_t)*num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint16_t * use_dst, const uint16_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(uint16_t)*num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void destroy(uint16_t * use_buf, _SizeTypeArg num_elem)
  {
#ifdef LW_ARRAY_DEBUG
    memset(use_buf, 0, sizeof(uint16_t)*num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void reset(uint16_t * use_buf, _SizeTypeArg num_elem)
  {
    memset(use_buf, 0, sizeof(uint16_t)*num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void copy(uint16_t * use_dst, const uint16_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(uint16_t)*num_elem);
  }
  //shifts number of elements to right
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(uint16_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf + shift_sz, (const void *)use_buf, sizeof(uint16_t)*num_elem);
  }
  //shifts number of elements to left
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(uint16_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf - shift_sz, (const void *)use_buf, sizeof(uint16_t)*num_elem);
  }
};


} //util
} //eyeline

#endif /* __LWARRAY_TRAITS_INT_DEFS */

