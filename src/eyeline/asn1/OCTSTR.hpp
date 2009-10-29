/* ************************************************************************* *
 * ASN.1 OCTET STRING type definitions.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OCTSTR_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_OCTSTR_DEFS__

#include <inttypes.h>
#include <sys/types.h>
#include "eyeline/util/LWArray.hpp"

namespace eyeline {
namespace asn1 {

static const uint8_t _OCTSTR_DFLT_SZ = 32;

//Default unbounded representation, though overall number of octets is limited to 64K
typedef eyeline::util::LWArray_T<uint8_t, uint16_t, _OCTSTR_DFLT_SZ> OCTSTR;

//Default constrained representation, overall number of octets is limited to 255
typedef eyeline::util::LWArray_T<uint8_t, uint8_t, _OCTSTR_DFLT_SZ> OCTSTR_ARRAYED;


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OCTSTR_DEFS__ */

