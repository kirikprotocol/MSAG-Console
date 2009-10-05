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
//Sets required kind of BER group encoding.
//Returns: encoding rule effective for this type.
//NOTE: may cause recalculation of TLVLayout due to restrictions
//      implied by the DER or CER
TSGroupBER::Rule_e EncoderOfRelativeOID::setRule(TSGroupBER::Rule_e use_rule)
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
TSLength  EncoderOfRelativeOID::calculateVAL(bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_valSZO && (!isIndefinite() || do_indef) 
      && !(_valSZO = estimate_SubIds(_encVal.get(), _encVal.size())))
    throw smsc::util::Exception("EncoderOfRelativeOID: illegal value");
  return _valSZO;
}

//Encodes by BER/DER/CER the type value ('V'-part of encoding)
//NOTE: Throws in case of value that cann't be encoded.
ENCResult EncoderOfRelativeOID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
{
  return encode_SubIds(_encVal.get(), _encVal.size(), use_enc, max_len);
}

} //ber
} //asn1
} //eyeline

