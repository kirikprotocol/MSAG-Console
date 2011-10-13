/* ************************************************************************** *
 * Various templates helping to manipulate with integer types signedness.
 * ************************************************************************** */
#ifndef __UITL_INTTYPES_UNSIGNER_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UITL_INTTYPES_UNSIGNER_T

#include <inttypes.h>

namespace smsc {
namespace util {

template <
  typename _IntT /* must be a integer type (signed or unsigned) */
> 
inline _IntT absOf(_IntT signed_val)
{
  return (signed_val >= 0) ? signed_val : -signed_val;
}

//Determines signedness of integer type
template <
  typename _IntT /* must be a integer type (signed or unsigned) */
> 
struct IsSignedInt_T {
  enum {
    RESULT = (((_IntT)(-1) < 0) ? 1 : 0)
  };
};

//Provides unsigned integer type of same sizeof()
template <
  typename _IntT  /* must be a integer type (signed or unsigned) */
>
struct IntUnsigner_T {
};

template < >
struct IntUnsigner_T<int8_t> {
  typedef uint8_t unsigned_type;
};
template < >
struct IntUnsigner_T<uint8_t> {
  typedef uint8_t unsigned_type;
};

template < >
struct IntUnsigner_T<int16_t> {
  typedef uint16_t unsigned_type;
};
template < >
struct IntUnsigner_T<uint16_t> {
  typedef uint16_t unsigned_type;
};

template < >
struct IntUnsigner_T<int32_t> {
  typedef uint32_t unsigned_type;
};
template < >
struct IntUnsigner_T<uint32_t> {
  typedef uint32_t unsigned_type;
};

template < >
struct IntUnsigner_T<int64_t> {
  typedef uint64_t unsigned_type;
};
template < >
struct IntUnsigner_T<uint64_t> {
  typedef uint64_t unsigned_type;
};


//Converts integer (signed or unsigned) from binary to decimal string
//form avoiding stdlib usage.
template <
  typename _IntT /* must be a integer type (signed or unsigned) */
> 
class IntToA_T {
protected:
  uint8_t  _idx;
  char     _buf[sizeof(_IntT)*3 + 2];

public:
  typedef typename IntUnsigner_T<_IntT>::unsigned_type unsigned_type;

  explicit IntToA_T(const _IntT use_val)
    : _idx(sizeof(_IntT)*3 + 1)
  {
    unsigned_type tVal = (unsigned_type)use_val;
    bool hasSign = false;
    if (IsSignedInt_T<_IntT>::RESULT) {
      unsigned_type mask = (1 << ((sizeof(_IntT)<<3) - 1));
      if ((hasSign = ((tVal & mask) != 0)))  //perform two's complement transformation
        tVal = ~tVal + 1;
    }
    _buf[_idx] = 0;
    do {
      _buf[--_idx] = '0' + (char)(tVal % 10);
    } while ((tVal /= 10) != 0);
    if (hasSign)
      _buf[--_idx] = '-';
  }
  ~IntToA_T()
  { }

  const char * get(void) const { return _buf + _idx; }
  uint8_t length(void) const { return sizeof(_IntT)*3 + 1 - _idx; }
};

}//util
}//smsc

#endif /* __UITL_INTTYPES_UNSIGNER_T */

