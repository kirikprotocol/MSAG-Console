#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

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
  DECResult rval(DECResult::decOk);

  if (!val_prop) {
    rval += skipTLV(use_enc, max_len, relaxed_rule);
  } else {
    if (val_prop->isDefinite()) {
      if (val_prop->_valLen < max_len)
        rval.status = DECResult::decMoreInput;
      else
        rval.nbytes = val_prop->_valLen;
    } else {
      rval += !val_prop->_isConstructed ? searchEOC(use_enc, max_len) :
                            searchEOCconstructed(use_enc, max_len, relaxed_rule);
    }
  }
  if (rval.isOk(relaxed_rule)) {
    _valDec->_rule = TSGroupBER::getTSRule(use_rule);
    _valDec->setPtr(use_enc);
    _valDec->_maxlen = rval.nbytes;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

