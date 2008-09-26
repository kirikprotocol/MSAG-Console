/* ************************************************************************* *
 * Templates helping to determine maximum size of several classes.
 * ************************************************************************* */
#ifndef __CPP_MAXSIZEOF_DEFS__
#ident "@(#)$Id$"
#define __CPP_MAXSIZEOF_DEFS__

namespace eyelinecom {
namespace util {

template <int a, int b>
struct MaxInt_T {
    enum {VALUE = a > b ? a : b};
}; 

template <class T1, class T2>
struct MaxSizeOf2_T {
    enum { VALUE = MaxInt_T<sizeof(T1),sizeof(T2)>::VALUE };
};

template <class T1, class T2, class T3>
struct MaxSizeOf3_T {
    enum { MAX12 = MaxInt_T<sizeof(T1),sizeof(T2)>::VALUE };
    enum { MAX23 = MaxInt_T<sizeof(T2),sizeof(T3)>::VALUE };
    enum { VALUE = MaxInt_T<MAX12, sizeof(T3)>::VALUE };
};

}; //util
}; //eyelinecom

#endif /* __CPP_MAXSIZEOF_DEFS__ */

