#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/ObjectID.hpp"

namespace eyeline {
namespace asn1 {

/* ************************************************************************* *
 * Validates value of OBJECT-IDENTIFER.
 * Returns  zero on success or ordinal number of invalid subId detected.
 * ************************************************************************* */
uint16_t  validate_ObjectID(const ObjectID & use_oid)
{
  //validate 1st subId
  if (use_oid[0] > 2)
    return 1; //illegal value

  //validate 2nd subId
  if ( (use_oid[1] > 39) && (!use_oid[0] || (use_oid[0] == 1)) )
    return 2; //illegal value

  //check that summ of first two subIds do not overload SubIdType
  SubIdType subId = use_oid[0]*40 +  use_oid[1];
  if (subId < use_oid[1])
    return 2; //too large value

  return 0; //Ok
}

} //asn1
} //eyeline

