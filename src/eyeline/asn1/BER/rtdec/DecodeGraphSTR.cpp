#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeGraphSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class GraphStrDecConverter implementation:
 * ************************************************************************* */
DECResult GraphStrDecConverter::unpackFragment(const uint8_t * use_buf, TSLength req_bytes) /* throw()*/
{
  std::string::size_type numOcts = TSBuffer::adoptRange<std::string::size_type>(req_bytes);
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

