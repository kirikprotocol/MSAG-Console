#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeBOOL.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the BOOLEAN value according to X.690 cl. 8.19
 * ************************************************************************* */
DECResult DecoderOfBOOL::decodeVAL(
                    const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::DecoderOfBOOL::decodeVal(): V-part properties isn't decoded");
  if (!_dVal)
    throw smsc::util::Exception("ber::DecoderOfBOOL::decodeVal(): value isn't set");

  TSLength  valLen = val_prop->_valLen;
  if (val_prop->isIndefinite()) { //check DER/CER restrictions
    if (!relaxed_rule && (use_rule != TSGroupBER::ruleBER))
      return DECResult(DECResult::decBadEncoding);

    if (!valLen) {
      DECResult reoc = searchEOC(use_enc, max_len);
      if (!reoc.isOk())
        return reoc;
      valLen = reoc.nbytes;
    }
  }
  if (valLen > 1)
    return DECResult(DECResult::decBadEncoding);
  //check DER/CER restrictions
  if (*use_enc && (*use_enc != 0xFF) && !relaxed_rule
      && ((use_rule == TSGroupBER::ruleCER)
          || (use_rule == TSGroupBER::ruleDER)))
    return DECResult(DECResult::decBadEncoding);

  *_dVal = (*use_enc != 0);
  return DECResult(DECResult::decOk, 1);
}

} //ber
} //asn1
} //eyeline


