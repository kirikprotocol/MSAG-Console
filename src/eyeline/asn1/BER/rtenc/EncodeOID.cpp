#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeOID.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSubIds.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfObjectID implementation:
 * ************************************************************************* */
uint16_t  EncoderOfObjectID::calculateSubIds(void) const /*throw(std::exception)*/
{
  uint8_t idx = _encVal.validate_ObjectID();
  if (idx)
    throw smsc::util::Exception("EncoderOfObjectID: illegal subId[%u]", idx-1);

  //process first two identifiers is special manner as stated in X.690 cl. 8.19.4
  SubIdType subId = _encVal[0]*40;
  if (_encVal.size() > 1)
    subId += _encVal[1];
  uint16_t rlen = estimate_SubIds(&subId, 1);

  //process rest of subIds
  if (rlen && _encVal.size() > 2)
    rlen += estimate_SubIds(_encVal.get() + 2, _encVal.size() - 2);

  if (!rlen)
    throw smsc::util::Exception("EncoderOfObjectID: illegal value");

  return rlen;
}

//Sets required kind of BER group encoding.
//Returns: encoding rule effective for this type.
//NOTE: may cause recalculation of TLVLayout due to restrictions
//      implied by the DER or CER
TSGroupBER::Rule_e EncoderOfObjectID::setRule(TSGroupBER::Rule_e use_rule)
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
TSLength  EncoderOfObjectID::calculateVAL(bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_valSZO && (!isIndefinite() || do_indef))
    _valSZO = calculateSubIds();
  return _valSZO;
}

//Encodes by BER/DER/CER the type value ('V'-part of encoding)
//NOTE: Throws in case of value that cann't be encoded.
ENCResult EncoderOfObjectID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
{
  uint8_t idx = _encVal.validate_ObjectID();
  if (idx)
    throw smsc::util::Exception("EncoderOfObjectID: illegal subId[%u]", idx-1);
  
  ENCResult rval(ENCResult::encOk);
  if (!_encVal.size()) {
    rval.status = ENCResult::encBadVal;
    return rval;
  }

  //encode first two identifiers is special manner as stated in X.690 cl. 8.19.4
  SubIdType subId = _encVal[0]*40;
  if (_encVal.size() > 1)
    subId += _encVal[1];
  rval += encode_SubIds(&subId, 1, use_enc, max_len);

  //encode rest of subIds
  if (rval.isOk() && _encVal.size() > 2)
    rval += encode_SubIds(_encVal.get() + 2, _encVal.size() - 2,
                          use_enc + rval.nbytes, max_len - rval.nbytes);
  return rval;
}


} //ber
} //asn1
} //eyeline

