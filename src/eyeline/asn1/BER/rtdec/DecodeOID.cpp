#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeOID.hpp"
#include "eyeline/asn1/BER/rtutl/DecodeIdents_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the OBJECT-IDENTIFIER value according to X.690 cl. 8.19
 * ************************************************************************* */
//NOTE: encoding of OBJECT-IDENTIFIER type value has the same form for all BER
//family rules: primitive encoding with definite LD form
DECResult DecoderOfObjectID::decodeVAL(const TLParser & tlv_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  if (!_dVal)
    throw smsc::util::Exception("ber::DecoderOfObjectID::decodeVal(): value isn't set");

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

  //decode 1st subid and split it to 1st and 2nd arcs ids
  asn1::SubIdType subId;
  DECResult rval = decode_identifier(subId, use_enc, valLen);
  if (!rval.isOk())
    return rval;

  if (subId < 80) { //arc 0 or 1
    (*_dVal)[0] = subId/40;
    (*_dVal)[1] = subId%40;
  } else {          //arc 2
    (*_dVal)[0] = 2;
    (*_dVal)[1] = subId - 80;
  }
  //decode rest of subIds
  asn1::ObjectID::size_type i = 2;
  while (rval.isOk() && (rval.nbytes <= valLen)) {
    rval += decode_identifier((*_dVal)[i], use_enc + rval.nbytes, valLen - rval.nbytes);
  }
  return rval;
}

} //ber
} //asn1
} //eyeline


