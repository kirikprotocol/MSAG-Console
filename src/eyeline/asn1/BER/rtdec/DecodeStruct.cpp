#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeStruct.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfStructAC implementation:
 * ************************************************************************* */

// -----------------------------------------------------------
// -- ValueDecoderIface interface methods
// -----------------------------------------------------------
DECResult
  DecoderOfStructAC::decodeVAL(const TLParser & tlv_prop, //corresponds to innermost identification tag
                             const uint8_t * use_enc, TSLength max_len,
                             TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                             bool relaxed_rule/* = false*/)
    /*throw(std::exception)*/
{
  DECResult rval(DECResult::decOk);

  if (tlv_prop.isDefinite())
    max_len = tlv_prop._valLen;

  _elDec->reset();
  do {
    TLParser  vtl;
    rval += vtl.decodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);

    if (rval.isOk(_relaxedRule))
      rval += decodeElement(vtl, use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
    /* */
  } while ((rval.nbytes < max_len) && rval.isOk(relaxed_rule)
           && (tlv_prop.isDefinite() || !checkEOC(use_enc + rval.nbytes, max_len - rval.nbytes)));

  if (rval.isOk(relaxed_rule)
      && (_elDec->verifyCompletion()._status != ElementDecoderAC::edaOk))
    rval.status = DECResult::decBadVal;
  return rval;
}

} //ber
} //asn1
} //eyeline

