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
DECResult DecoderOfASType::decodeVAL(const TLParser & tlv_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/) /*throw(std::exception)*/
{
  uint16_t  shift = 0;
  DECResult rval(DECResult::decOk);

  _valDec->_rule = TSGroupBER::getTSRule(use_rule);
  if (isTagged()) {
    rval += skipTLV(use_enc, max_len, relaxed_rule);
  } else { //untagged Opentype, identification tag is a part of value encoding
    rval += skipTLV(tlv_prop, use_enc, max_len, relaxed_rule);
    shift = tlv_prop.getBOCsize();
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

