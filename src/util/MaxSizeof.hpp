/* ************************************************************************* *
 * Templates helping to determine maximum size of several classes.
 * ************************************************************************* */
#ifndef __UTIL_CPP_MAXSIZEOF_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_CPP_MAXSIZEOF_DEFS__

#include <sys/types.h>

namespace smsc {
namespace util {

template <int a, int b>
struct MaxInt_T {
  enum { VALUE = a > b ? a : b};
}; 
template <int a, int b, int c>
struct MaxIntOf3_T {
  enum { MAX12 = MaxInt_T<a, b>::VALUE };
  enum { VALUE = MaxInt_T<MAX12, c>::VALUE };
}; 


template <class T1, class T2>
struct MaxSizeOf2_T {
  enum { VALUE = MaxInt_T<sizeof(T1),sizeof(T2)>::VALUE };
};

template <class T1, class T2, class T3>
struct MaxSizeOf3_T {
  enum { MAX12 = MaxInt_T<sizeof(T1), sizeof(T2)>::VALUE };
  enum { MAX23 = MaxInt_T<sizeof(T2), sizeof(T3)>::VALUE };
  enum { VALUE = MaxInt_T<MAX12, sizeof(T3)>::VALUE };
};

template <class T1, class T2, class T3, class T4>
struct MaxSizeOf4_T {
  enum { MAX12 = MaxInt_T<sizeof(T1), sizeof(T2)>::VALUE };
  enum { MAX34 = MaxInt_T<sizeof(T3), sizeof(T4)>::VALUE };
  enum { VALUE = MaxInt_T<MAX12, MAX34>::VALUE };
};

template <class T1, class T2, class T3, class T4, class T5>
struct MaxSizeOf5_T {
  enum { MAX12 = MaxInt_T<sizeof(T1), sizeof(T2)>::VALUE };
  enum { MAX34 = MaxInt_T<sizeof(T3), sizeof(T4)>::VALUE };
  enum { VALUE = MaxIntOf3_T<MAX12, MAX34, sizeof(T5)>::VALUE };
};

template <class T1, class T2, class T3, class T4, class T5, class T6>
struct MaxSizeOf6_T {
  enum { MAX12 = MaxInt_T<sizeof(T1), sizeof(T2)>::VALUE };
  enum { MAX34 = MaxInt_T<sizeof(T3), sizeof(T4)>::VALUE };
  enum { MAX56 = MaxInt_T<sizeof(T5), sizeof(T6)>::VALUE };
  enum { VALUE = MaxIntOf3_T<MAX12, MAX34, MAX56>::VALUE };
};

template <size_t _SZOArg, class _TArg>
struct MultiplierOfSize_T {
  enum { VALUE = (_SZOArg + sizeof(_TArg) -1)/sizeof(_TArg) };
};

} //smsc
} //eyeline

#endif /* __UTIL_CPP_MAXSIZEOF_DEFS__ */

