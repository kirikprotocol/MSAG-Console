/* ************************************************************************* *
 * ASN.1 OBJECT-IDENTIFIER & RELATIVE-OID types.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_OID_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_OID_DEFS__

#include <inttypes.h>
#include <vector>

namespace eyeline {
namespace asn1 {

typedef uint16_t SubIdType;

typedef std::vector<SubIdType> ObjectID;
typedef std::vector<SubIdType> RelativeOID;

/* ************************************************************************* *
 * Validates value of OBJECT-IDENTIFER.
 * Returns  zero on success or ordinal number of invalid subId detected.
 * ************************************************************************* */
extern uint16_t  validate_ObjectID(const ObjectID & use_oid);

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_OID_DEFS__ */

