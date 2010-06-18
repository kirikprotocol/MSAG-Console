#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeROID.hpp"
#include "eyeline/asn1/BER/rtutl/DecodeIdents_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the RELATIVE-OID value according to X.690 cl. 8.20
 * ************************************************************************* */
//NOTE: encoding of RelativeOID type value has the same form for all BER
//family rules: primitive encoding with definite LD form

DECResult DecoderOfRelativeOID::decodeVAL(
                    const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::DecoderOfRelativeOID::decodeVal(): V-part properties isn't decoded");
  if (!_dVal)
    throw smsc::util::Exception("ber::DecoderOfRelativeOID::decodeVal(): value isn't set");

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
  //decode sequence of subIds
  asn1::ObjectID::size_type i = 0;
  while (rval.isOk() && (rval.nbytes <= valLen)) {
    rval += decode_identifier((*_dVal)[i], use_enc + rval.nbytes, valLen - rval.nbytes);
  }
  return rval;
}

} //ber
} //asn1
} //eyeline


