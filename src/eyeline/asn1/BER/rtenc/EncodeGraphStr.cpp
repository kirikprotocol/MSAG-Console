#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeGraphStr.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class GraphSTREncConverter implementation:
 * ************************************************************************* */
//Incrementally converts next portion of string value writing requested
//number of bytes to provided buffer.
//Returns number of bytes written to buffer.
TSLength GraphSTREncConverter::pack2Octs(uint8_t *use_buf, TSLength req_bytes)
{
  if (req_bytes > (_encValSz - _numConverted))
    req_bytes = _encValSz - _numConverted;
  memcpy(use_buf, _encVal + _numConverted, req_bytes);
  _numConverted += req_bytes;
  return req_bytes;
}

} //ber
} //asn1
} //eyeline

