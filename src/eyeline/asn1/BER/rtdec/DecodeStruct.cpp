#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeStruct.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfStructAC implementation:
 * ************************************************************************* */
DECResult DecoderOfStructAC::decodeElement(
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
    if (!tDec)
      rval.status = DECResult::decBadVal;
    else {
      if (edRes._alt->getEnv() != EDAlternative::atgExplicit)
        tDec->setOutermostTL(vtl);
      rval += tDec->decode(use_enc + rval.nbytes, max_len - rval.nbytes);

      if (rval.isOk(relaxed_rule)) {
        if (edRes._alt->isOptional())
          markDecodedOptional(edRes._alt->getUId());
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
  }
  return rval;
}


DECResult
  DecoderOfStructAC::decodeVAL(const TLVProperty * val_prop,
                             const uint8_t * use_enc, TSLength max_len,
                             TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                             bool relaxed_rule/* = false*/)
    /*throw(std::exception)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::DecoderOfStructAC::decodeVal(): V-part properties isn't decoded");

  DECResult rval(DECResult::decOk);

  if (val_prop->isDefinite()) {
    max_len = val_prop->_valLen;
    do {
      rval += decodeElement(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
    } while ((rval.nbytes < max_len) && rval.isOk(relaxed_rule));
    /**/
  } else {
    //indefinite form
    while (rval.nbytes < max_len) {
      rval += decodeElement(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
      if (!rval.isOk(relaxed_rule))
        break;
      if (checkEOC(use_enc + rval.nbytes, max_len - rval.nbytes)) {
        rval.nbytes += 2; break;
      }
    }
  }
  if (rval.isOk(relaxed_rule)
      && (_elDec->verifyCompletion()._status != ElementDecoderAC::edaOk))
    rval.status = DECResult::decBadVal;
  return rval;
}


} //ber
} //asn1
} //eyeline

