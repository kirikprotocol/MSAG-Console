#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
  if (!_encVal->size())
    throw smsc::util::Exception("EncoderOfObjectID: illegal value");

  uint8_t idx = _encVal->validate_ObjectID();
  if (idx)
    throw smsc::util::Exception("EncoderOfObjectID: illegal subId[%u]", idx-1);

  //process first two identifiers is special manner as stated in X.690 cl. 8.19.4
  SubIdType subId = (*_encVal)[0]*40;
  if (_encVal->size() > 1)
    subId += (*_encVal)[1];
  uint16_t rlen = estimate_SubIds(&subId, 1);

  //process rest of subIds
  if (rlen && _encVal->size() > 2)
    rlen += estimate_SubIds(_encVal->get() + 2, _encVal->size() - 2);

  if (!rlen)
    throw smsc::util::Exception("EncoderOfObjectID: illegal value");

  return rlen;
}

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
//NOTE: encoding of ObjectID type value has the same form for all BER
//family rules: primitive encoding with definite LD form

void
  EncoderOfObjectID::calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                                  bool do_indef/* = false*/) /*throw(std::exception)*/
{
  if (!_encVal)
    throw smsc::util::Exception("ber::EncoderOfOID: value isn't set");

  val_prop._valLen = calculateSubIds();
  val_prop._ldForm = LDeterminant::frmDefinite;
  val_prop._isConstructed = false;
}

ENCResult
  EncoderOfObjectID::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  uint8_t idx = _encVal->validate_ObjectID();
  if (idx)
    throw smsc::util::Exception("EncoderOfObjectID: illegal subId[%u]", idx-1);
  
  ENCResult rval(ENCResult::encOk);
  if (!_encVal->size()) {
    rval.status = ENCResult::encBadVal;
    return rval;
  }

  //encode first two identifiers is special manner as stated in X.690 cl. 8.19.4
  SubIdType subId = (*_encVal)[0]*40;
  if (_encVal->size() > 1)
    subId += (*_encVal)[1];
  rval += encode_SubIds(&subId, 1, use_enc, max_len);

  //encode rest of subIds
  if (rval.isOk() && _encVal->size() > 2)
    rval += encode_SubIds(_encVal->get() + 2, _encVal->size() - 2,
                          use_enc + rval.nbytes, max_len - rval.nbytes);
  return rval;
}


} //ber
} //asn1
} //eyeline

