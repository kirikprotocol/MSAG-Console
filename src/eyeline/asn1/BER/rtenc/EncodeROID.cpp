#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeROID.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSubIds.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfRelativeOID implementation:
 * ************************************************************************* */

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
//NOTE: encoding of INTEGER type value has the same form for all BER
//family rules: primitive encoding with definite LD form

const EncodingProperty &
  EncoderOfRelativeOID::calculateVAL(bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!(_vProp._valLen = estimate_SubIds(_encVal.get(), _encVal.size())))
    throw smsc::util::Exception("EncoderOfRelativeOID: illegal value");
  return _vProp;
}

ENCResult
  EncoderOfRelativeOID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
{
  return encode_SubIds(_encVal.get(), _encVal.size(), use_enc, max_len);
}

} //ber
} //asn1
} //eyeline

