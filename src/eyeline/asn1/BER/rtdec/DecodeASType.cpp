#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
/* ************************************************************************* *
 * Class DecoderOfASType implementation:
 * ************************************************************************* */
DECResult DecoderOfASType::decodeVAL(const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(BERDecoderException)*/
{
  uint16_t  shift = 0;
  DECResult rval(DECResult::decOk);

  _valDec->_rule = TSGroupBER::getTSRule(use_rule);
  if (isTagged()) {
    if (!val_prop)
      throw smsc::util::Exception("ber::DecoderOfASType::decodeVal(): V-part properties isn't decoded");

    rval += skipTLV(use_enc, max_len, relaxed_rule);
  } else { //untagged Opentype
    if (!val_prop) { //Opentype is a PDU
      rval += skipTLV(use_enc, max_len, relaxed_rule);
    } else { // _outerTL is set and is a part of Opentype encoding
      rval += skipTLV(*_outerTL, use_enc, max_len, relaxed_rule);
      shift = _outerTL->getBOCsize();
    }
  }

  if (rval.isOk(relaxed_rule)) {
    _valDec->setPtr(use_enc - shift);
    _valDec->_maxlen = rval.nbytes + shift;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

