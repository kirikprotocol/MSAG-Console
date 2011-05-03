/* ************************************************************************** *
 * Templates providing conversion from any integer type to unsigned type 
 * of same sizeof().
 * ************************************************************************** */
#ifndef __UITL_INTTYPES_UNSIGNER_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UITL_INTTYPES_UNSIGNER_T

#include <inttypes.h>

namespace smsc {
namespace util {

template < typename _IntT >
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

}//util
}//smsc

#endif /* __UITL_INTTYPES_UNSIGNER_T */

