#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the OBJECT-IDENTIFIER value according to X.690 cl. 8.19
 * ************************************************************************* */
DECResult DecoderOfEOID::decodeVAL(
                    const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::DecoderOfEOID::decodeVal(): V-part properties isn't decoded");
  if (!_dVal)
    throw smsc::util::Exception("ber::DecoderOfEOID::decodeVal(): value isn't set");

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

  DECResult rval(DECResult::decOk);
  if (valLen > (uint16_t)(-1))
    rval.status = DECResult::decBadVal; //too long value
  else {
    _dVal->init((uint16_t)valLen, use_enc);
    rval.nbytes = valLen;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline


