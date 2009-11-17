#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfSequenceAC implementation:
 * ************************************************************************* */
TSLength 
  EncoderOfSequenceAC::calculateFields(bool calc_indef/* = false*/) const
    /*throw(std::exception)*/
{
  bool      definite = true;
  TSLength  valLen = 0;

  for (uint16_t i = 0; i < _fldCnt; ++i) {
    getField(i).setRule(getRule());
    const TLVProperty & tlvVal = getField(i).calculateTLV(calc_indef);
    if (definite && tlvVal.getTLVsize())
      valLen += tlvVal.getTLVsize();
    else {
      valLen = 0;
      definite = false;
    }
  }
  return valLen;
}

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
bool EncoderOfSequenceAC::setRule(TSGroupBER::Rule_e use_rule) const
{
  for (uint16_t i = 0; i < _fldCnt; ++i) {
    if (getField(i).setRule(use_rule))
      _isCalculated = false;
  }
  return !_isCalculated;
}

const EncodingProperty &
  EncoderOfSequenceAC::calculateVAL(bool calc_indef/* = false*/) const
    /*throw(std::exception)*/
{
  _vProp._isConstructed = true;
  if (getRule() == TSGroupBER::ruleCER) {
    _vProp._ldForm = LDeterminant::frmIndefinite;
    _vProp._valLen = calculateFields(calc_indef);
  } else if (getRule() == TSGroupBER::ruleDER) {
    _vProp._ldForm = LDeterminant::frmDefinite;
    _vProp._valLen = calculateFields(true);
  } else {
    _vProp._valLen = calculateFields(calc_indef);
    _vProp._ldForm = _vProp._valLen ? LDeterminant::frmDefinite :
                                      LDeterminant::frmIndefinite;
  }
  _isCalculated = true;
  return _vProp;
}

ENCResult
  EncoderOfSequenceAC::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);
  for (uint16_t i = 0; i < _fldCnt; ++i) {
    rval += getField(i).encodeTLV(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

