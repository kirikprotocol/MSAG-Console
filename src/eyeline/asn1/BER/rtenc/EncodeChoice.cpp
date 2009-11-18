#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfChoice implementation:
 * ************************************************************************* */

// -- ************************************* --
// -- ValueEncoderAC interface methods
// -- ************************************* --
bool EncoderOfChoice::setRule(TSGroupBER::Rule_e use_rule)
{
  if (_alt.setRule(use_rule))
    _isCalculated = false;
  return !_isCalculated;
}

const EncodingProperty &
  EncoderOfChoice::calculateVAL(bool calc_indef/* = false*/) /*throw(std::exception)*/
{
  _alt.setRule(getRule());
  const TLVProperty & tlvVal = _alt.calculateTLV(calc_indef);
  _vProp._isConstructed = false;
  _vProp._valLen = tlvVal.getTLVsize();
  _vProp._ldForm = tlvVal._ldForm;
  _isCalculated = true;
  return _vProp;
}

} //ber
} //asn1
} //eyeline

