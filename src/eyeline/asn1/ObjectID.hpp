/* ************************************************************************* *
 * ASN.1 OBJECT-IDENTIFIER & RELATIVE-OID types.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OID_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_OID_DEFS__

#include <inttypes.h>
#include "core/buffers/LWArrayT.hpp"
#include "core/buffers/LWArrayTraitsInt.hpp"

namespace eyeline {
namespace asn1 {

typedef uint16_t SubIdType;

static const uint8_t _ObjectID_DFLT_SUBIDS_NUM = 16;
//NOTE: Overall number of subIdentifiers is limited to 255
static const uint8_t _ObjectID_MAX_SUBIDS_NUM = 255;

typedef smsc::core::buffers::LWArray_T<SubIdType, uint8_t, _ObjectID_DFLT_SUBIDS_NUM> RelativeOID;

class ObjectID : public RelativeOID {
public:
  // Validates value of OBJECT-IDENTIFER type.
  // Returns zero on success or ordinal number of invalid subId detected.
  uint8_t  validate_ObjectID(void) const;
};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OID_DEFS__ */

