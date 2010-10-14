#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfChoiceAC implementation:
 * ************************************************************************* */
DECResult DecoderOfChoiceAC::decodeElement(
                                const uint8_t * use_enc, TSLength max_len,
                                bool relaxed_rule)
  /*throw(std::exception)*/
{
  TLParser  vtl;
  DECResult rval = vtl.decodeBOC(use_enc, max_len);
  if (!rval.isOk(relaxed_rule))
    return rval;

  ElementDecoderAC::EDAResult edRes = _elDec->processElement(vtl._tag);
  if (!edRes.isOk()) {
    rval.status = DECResult::decBadEncoding;
  } else {
    TypeDecoderAC * tDec =  prepareAlternative(edRes._alt->getUId());

    if (edRes._alt->getEnv() != EDAlternative::atgExplicit)
      tDec->setOutermostTL(vtl);
    rval += tDec->decode(use_enc + rval.nbytes, max_len - rval.nbytes);

    if (rval.isOk(relaxed_rule)) {
      markDecodedAlternative(edRes._alt->getUId());
      //in case of explicitly tagged alternative eat EOC's 
      if ((edRes._alt->getEnv() == EDAlternative::atgExplicit)
          && vtl.isIndefinite()) {
        if (checkEOC(use_enc + rval.nbytes, max_len - rval.nbytes))
          rval.nbytes += 2; 
        else
          rval.status = DECResult::decBadEncoding;
      }
    }
  }
  return rval;
}

//NOTE: in case of untagged CHOICE, val_prop may be == 0
DECResult DecoderOfChoiceAC::decodeVAL(
                      const TLVProperty * val_prop,
                      const uint8_t * use_enc, TSLength max_len,
                      TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                      bool relaxed_rule/* = false*/)
    /*throw(std::exception)*/
{
  if (val_prop && val_prop->isDefinite())
    max_len = val_prop->_valLen;

  DECResult rval = decodeElement(use_enc, max_len, relaxed_rule);

  if (val_prop && val_prop->isIndefinite()) {
    if (checkEOC(use_enc + rval.nbytes, max_len - rval.nbytes))
      rval.nbytes += 2; 
    else
      rval.status = DECResult::decBadEncoding;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

