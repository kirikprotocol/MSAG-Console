#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/EXT/enc/MEPCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

void
MEPCS_Extensions::setValue(const PCS_Extensions& value)
{
  clearFields(0);
  if (!value._unkExt._tsList.empty())
    setExtensions(value._unkExt, 0);
}

}}}}
