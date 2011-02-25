#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfNULL implementation:
 * ************************************************************************* */

//NOTE: encoding of NULL type value has the same form for all BER
//family rules: primitive encoding(0b) with definite LD form

// -- ************************************* --
// -- ValueDecoderIface interface methods
// -- ************************************* --

DECResult DecoderOfNULL::decodeVAL(const TLParser & tlv_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(BERDecoderException)*/
{
  DECResult rval(DECResult::decBadEncoding);
  TSLength  valLen = tlv_prop._valLen;

  if (tlv_prop.isIndefinite()) { //check DER/CER restrictions
    if (!relaxed_rule && (use_rule != TSGroupBER::ruleBER))
      return rval;

    DECResult reoc = searchEOC(use_enc, max_len);
    if (!reoc.isOk())
      return reoc;
    valLen = reoc.nbytes;
  }
  if (!valLen)
    rval.status = DECResult::decOk;
  return rval;
}


} //ber
} //asn1
} //eyeline

