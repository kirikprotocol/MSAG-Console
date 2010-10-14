#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeConstructed.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class EncoderOfConstructedAC implementation:
 * ************************************************************************* */
TSLength EncoderOfConstructedAC::calculateElements(bool calc_indef/* = false*/)
    /*throw(std::exception)*/
{
  bool      definite = true;
  TSLength  valLen = 0;
  TLVLayoutEncoder * pElem = 0;

  for (uint16_t i = 0; (pElem = getNextElement(i)); ++i) {
    const TLVStruct & tlvVal = pElem->calculateTLV(calc_indef);
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
// -- ValueEncoderIface interface methods
// -- ************************************* --

void EncoderOfConstructedAC::calculateVAL(TLVProperty & val_prop,
                                          TSGroupBER::Rule_e use_rule,
                                          bool calc_indef/* = false*/)
    /*throw(std::exception)*/
{
  val_prop._isConstructed = true;
  if (getVALRule() == TSGroupBER::ruleCER) {
    val_prop._ldForm = LDeterminant::frmIndefinite;
    val_prop._valLen = calculateElements(calc_indef);
  } else if (getVALRule() == TSGroupBER::ruleDER) {
    val_prop._ldForm = LDeterminant::frmDefinite;
    val_prop._valLen = calculateElements(true);
  } else {
    val_prop._valLen = calculateElements(calc_indef);
    val_prop._ldForm = val_prop._valLen ? LDeterminant::frmDefinite :
                                      LDeterminant::frmIndefinite;
  }
}

ENCResult
  EncoderOfConstructedAC::encodeVAL(uint8_t * use_enc, TSLength max_len) const
    /*throw(std::exception)*/
{
  ENCResult rval(ENCResult::encOk);
  const TLVLayoutEncoder * pElem = 0;

  for (uint16_t i = 0; (pElem = getNextElement(i)); ++i) {
    rval += pElem->encodeTLV(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}


} //ber
} //asn1
} //eyeline

