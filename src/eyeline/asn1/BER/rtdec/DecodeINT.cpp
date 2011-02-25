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
DECResult DecoderOfINTEGER::decodeVAL(const TLParser & tlv_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!_pVal.u8)
    throw smsc::util::Exception("ber::ValueDecoderOfINTEGER::decodeVal(): value isn't set");

  TSLength  valLen = tlv_prop._valLen;
  if (tlv_prop.isIndefinite()) { //check DER/CER restrictions
    if (!relaxed_rule && (use_rule != TSGroupBER::ruleBER))
      return DECResult(DECResult::decBadEncoding);

    if (!valLen) {
      DECResult reoc = searchEOC(use_enc, max_len);
      if (!reoc.isOk())
        return reoc;
      valLen = reoc.nbytes;
    }
  }
  if (!valLen)
    return DECResult(DECResult::decBadEncoding);

  if ((*use_enc & 0x80) && !_vSigned) //positive value only is expected
    return DECResult(DECResult::decBadVal);


  DECResult rval(DECResult::decOk);
  switch (_vSzo) {
  case szo32: {
    rval = _vSigned ? decodeIntCOC_signed(*_pVal.i32, use_enc, valLen)
                    : decodeIntCOC_unsigned(*_pVal.u32, use_enc, valLen);
  } break;

  case szo16: {
    rval = _vSigned ? decodeIntCOC_signed(*_pVal.i16, use_enc, valLen)
                    : decodeIntCOC_unsigned(*_pVal.u16, use_enc, valLen);
  } break;

  default: //szo8
    rval = _vSigned ? decodeIntCOC_signed(*_pVal.i8, use_enc, valLen)
                    : decodeIntCOC_unsigned(*_pVal.u8, use_enc, valLen);
  }
  if ((rval.status == DECResult::decOkRelaxed) && !relaxed_rule)
    return DECResult(DECResult::decBadEncoding);
  return rval;
}

} //ber
} //asn1
} //eyeline


