/* ************************************************************************* *
 * ASN.1 INTEGER type.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_INT_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_INT_DEFS__

#include <inttypes.h>

namespace eyeline {
namespace asn1 {

typedef uint32_t INTEGER;  //default INTEGER type
//typedef uint64_t INTEGER_LONG;
//typedef LWArray_T<uint8_t, uint8_t, 24> INTEGER_HUGE;

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_INT_DEFS__ */

