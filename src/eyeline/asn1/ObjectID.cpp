#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/ObjectID.hpp"

namespace eyeline {
namespace asn1 {

/* ************************************************************************* *
 * Validates value of OBJECT-IDENTIFER.
 * Returns  zero on success or ordinal number of invalid subId detected.
 * ************************************************************************* */
uint8_t  ObjectID::validate_ObjectID(void) const
{
  if (size()) {
    //validate 1st subId
    if (get()[0] > 2)
      return 1; //illegal value

    if (size() > 1) {
      //validate 2nd subId
      if ( (get()[1] > 39) && (!get()[0] || (get()[0] == 1)) )
        return 2; //illegal value

      //check that summ of first two subIds do not overload SubIdType
      SubIdType subId = get()[0]*40 +  get()[1];
      if (subId < get()[1])
        return 2; //too large value
    }
  }
  return 0; //Ok
}

} //asn1
} //eyeline

