#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeInt_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfENUM implementation:
 * ************************************************************************* */
//Sets required kind of BER group encoding.
//Returns: encoding rule effective for this type.
//NOTE: may cause recalculation of TLVLayout due to restrictions
//      implied by the DER or CER
TSGroupBER::Rule_e EncoderOfENUM::setRule(TSGroupBER::Rule_e use_rule)
{
  if (_rule != use_rule) {
    _rule = use_rule;
    _tldSZO = 0;
  }
  return _rule;
}

//Calculates length of BER/DER/CER encoding of type value.
//If 'do_indef' flag is set, then length of value encoding is computed
//even if TLVLayout uses only indefinite form of length determinants.
//NOTE: may change TLVLayout if type value encoding should be fragmented.
//NOTE: Throws in case of value that cann't be encoded.
TSLength  EncoderOfENUM::calculateVAL(bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_valSZO && (!isIndefinite() || do_indef))
    _valSZO = estimate_INTEGER(_encVal);
  return _valSZO;
}

//Encodes by BER/DER/CER the type value ('V'-part of encoding)
//NOTE: Throws in case of value that cann't be encoded.
ENCResult EncoderOfENUM::encodeVAL(uint8_t * use_enc, TSLength max_len) const
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

