/* ************************************************************************* *
 * Lightweight Array POD-element traits: specializations for integer types.
 * ************************************************************************* */
#ifndef __CORE_BUFFERS_LWARRAY_TRAITS_INT_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_LWARRAY_TRAITS_INT_DEFS

#include <inttypes.h>
#include "core/buffers/LWArrayTraitsT.hpp"

namespace smsc {
namespace core {
namespace buffers {

template </* _TArg = uint8_t */>
struct LWArrayTraitsPOD_T<uint8_t> {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint8_t * use_dst, const uint8_t & use_val, _SizeTypeArg num_elem)
  {
    memset(use_dst, use_val, num_elem);
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
  static void assign(uint8_t * use_buf, _SizeTypeArg num_elem, const uint8_t & use_val)
  {
    memset(use_buf, use_val, num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(uint8_t * use_dst, const uint8_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static short compare(const uint8_t * use_dst, const uint8_t * use_src, _SizeTypeArg num_elem)
  {
    return (short)memcmp(use_dst, use_src, num_elem);
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
struct LWArrayTraitsPOD_T<uint16_t> {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint16_t * use_dst, const uint16_t & use_val, _SizeTypeArg num_elem)
  {
    if (!use_val) {
      memset(use_dst, 0x00, sizeof(uint16_t)*num_elem);
    } else {
      for (_SizeTypeArg i = 0; i < num_elem; ++i) {
        use_dst[i] = use_val;
      }
    }
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
  static void assign(uint16_t * use_buf, _SizeTypeArg num_elem, const uint16_t & use_val)
  {
    construct(use_buf, num_elem, use_val);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(uint16_t * use_dst, const uint16_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(uint16_t)*num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static short compare(const uint16_t * use_dst, const uint16_t * use_src, _SizeTypeArg num_elem)
  {
    return (short)memcmp(use_dst, use_src, sizeof(uint16_t)*num_elem);
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


template </* _TArg = uint32_t */>
struct LWArrayTraitsPOD_T<uint32_t> {
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint32_t * use_dst, const uint32_t & use_val, _SizeTypeArg num_elem)
  {
    if (!use_val) {
      memset(use_dst, 0x00, sizeof(uint32_t)*num_elem);
    } else {
      for (_SizeTypeArg i = 0; i < num_elem; ++i) {
        use_dst[i] = use_val;
      }
    }
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void construct(uint32_t * use_dst, const uint32_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(uint32_t)*num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void destroy(uint32_t * use_buf, _SizeTypeArg num_elem)
  {
#ifdef LW_ARRAY_DEBUG
    memset(use_buf, 0xFF, sizeof(uint32_t)*num_elem);
#endif /* LW_ARRAY_DEBUG */
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(uint32_t * use_buf, _SizeTypeArg num_elem, const uint32_t & use_val)
  {
    construct(use_buf, num_elem, use_val);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void assign(uint32_t * use_dst, const uint32_t * use_src, _SizeTypeArg num_elem)
  {
    memcpy(use_dst, use_src, sizeof(uint32_t)*num_elem);
  }
  //
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static short compare(const uint32_t * use_dst, const uint32_t * use_src, _SizeTypeArg num_elem)
  {
    return (short)memcmp(use_dst, use_src, sizeof(uint32_t)*num_elem);
  }

  //shifts number of elements to right
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_right(uint32_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf + shift_sz, (const void *)use_buf, sizeof(uint32_t)*num_elem);
  }
  //shifts number of elements to left
  template < typename _SizeTypeArg /* must be an unsigned integer type */ >
  static void shift_left(uint32_t * use_buf, _SizeTypeArg num_elem, const _SizeTypeArg shift_sz)
  {
    memmove(use_buf - shift_sz, (const void *)use_buf, sizeof(uint32_t)*num_elem);
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_LWARRAY_TRAITS_INT_DEFS */

