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

DECResult DecoderOfNULL::decodeVAL(const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(BERDecoderException)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::DecoderOfNULL::decodeVAL(): V-part properties isn't decoded");

  DECResult rval(DECResult::decBadEncoding);
  TSLength  valLen = val_prop->_valLen;

  if (val_prop->isIndefinite()) { //check DER/CER restrictions
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

