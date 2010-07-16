#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
/* ************************************************************************* *
 * Class DecoderOfUExtension implementation:
 * ************************************************************************* */
DECResult DecoderOfUExtension::decodeVAL(const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(BERDecoderException)*/
{
  _valDec->_tsList.push_back(TransferSyntax());
  DecoderOfASType extDec(TSGroupBER::getTSRule(use_rule));
  extDec.setValue(_valDec->_tsList.back());
  return extDec.decode(use_enc, max_len);
}

} //ber
} //asn1
} //eyeline

