#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtutl/DecodeIntCOC_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the integer value according to X.690 cl. 8.3
 * ************************************************************************* */
DECResult DecoderOfINTEGER::decodeVAL(
                    const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::ValueDecoderOfINTEGER::decodeVal(): V-part properties isn't decoded");
  if (!_pVal.u8)
    throw smsc::util::Exception("ber::ValueDecoderOfINTEGER::decodeVal(): value isn't set");

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
  switch (_vSzo) {
  case szo32: {
    rval = decodeCOC_UINTEGER(*_pVal.u32, use_enc, valLen);
  } break;

  case szo16: {
    rval = decodeCOC_UINTEGER(*_pVal.u16, use_enc, valLen);
  } break;

  default: //szo8
    rval = decodeCOC_UINTEGER(*_pVal.u8, use_enc, valLen);
  }

  if ((rval.status == DECResult::decOkRelaxed) && relaxed_rule && (rval.nbytes < valLen))
    rval.status = DECResult::decBadVal;
  return rval;
}

} //ber
} //asn1
} //eyeline


