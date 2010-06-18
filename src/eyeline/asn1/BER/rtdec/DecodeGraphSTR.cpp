#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeGraphSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class GraphStrDecConverter implementation:
 * ************************************************************************* */
DECResult GraphStrDecConverter::unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/
{
  std::string::size_type numOcts = DOWNCAST_UNSIGNED(req_bytes, std::string::size_type);
  DECResult rval(DECResult::decOk);

  try {
    _dVal->reserve(numOcts);
    _dVal->append((const char*)use_buf, numOcts);
    rval.nbytes = (TSLength)numOcts;
  } catch (...) {
    rval.status = DECResult::decBadVal;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

