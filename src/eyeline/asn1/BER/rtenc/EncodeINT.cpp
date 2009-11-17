#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeInt_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfINTEGER implementation:
 * ************************************************************************* */
//NOTE: encoding of INTEGER type value has the same form for all BER
//family rules: primitive encoding with definite LD form

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --

const EncodingProperty &
  EncoderOfINTEGER::calculateVAL(bool do_indef/* = false*/) const /*throw(std::exception)*/
{
  _vProp._valLen = estimate_INTEGER(_encVal);
  _vProp._ldForm = LDeterminant::frmDefinite;
  _vProp._isConstructed = false;
  _isCalculated = true;
  return _vProp;
}

ENCResult 
  EncoderOfINTEGER::encodeVAL(uint8_t * use_enc, TSLength max_len) const
{
  ENCResult rval(ENCResult::encOk);
  uint8_t nb = encode_INTEGER(_encVal, use_enc, max_len);
  if (!nb)
    rval.status = ENCResult::encMoreMem;
  else
    rval.nbytes += nb;
  return rval;
}

} //ber
} //asn1
} //eyeline

