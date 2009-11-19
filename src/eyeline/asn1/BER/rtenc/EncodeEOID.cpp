#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfEOID implementation:
 * ************************************************************************* */

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
//NOTE: encoding of ObjectID type value has the same form for all BER
//family rules: primitive encoding with definite LD form

const EncodingProperty &
  EncoderOfEOID::calculateVAL(bool do_indef/* = false*/) /*throw(std::exception)*/
{
  _vProp._valLen = _encVal.length();
  _vProp._ldForm = LDeterminant::frmDefinite;
  _vProp._isConstructed = false;
  _isCalculated = true;
  return _vProp;
}

ENCResult
  EncoderOfEOID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  if (max_len < _encVal.length()) {
    rval.status = ENCResult::encMoreMem;
  } else {
    memcpy(use_enc, _encVal.octets(), rval.nbytes = _encVal.length());
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

