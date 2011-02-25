#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeConstructed.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfConstructedAC implementation:
 * ************************************************************************* */
DECResult DecoderOfConstructedAC::decodeElement(const TLParser & tlv_prop,
                                const uint8_t * use_enc, TSLength max_len,
                                bool relaxed_rule)
  /*throw(std::exception)*/
{
  DECResult                     rval(DECResult::decOk);
  ElementDecoderAC::EDAResult   edRes = _elDec->processElement(tlv_prop._tag);

  if (!edRes.isOk()) {
    rval.status = DECResult::decBadEncoding;
  } else {
    TypeDecoderAC * tDec =  prepareAlternative(edRes._alt->getUId());
    if (!tDec)
      rval.status = DECResult::decBadVal;
    else {
      bool wrappingTag = false;

      switch (edRes._alt->getEnv()) {
      case EDAlternative::atgUntagged: {
        //wrappingTag = false;
        tDec->setOutermostTL(&tlv_prop);
      } break;

      case EDAlternative::atgImplicit: {
        if (tDec->isTagged())
          tDec->setOutermostTL(&tlv_prop);
        else  //untagged CHOICE/ANY/OpenType: outer IMPLICT tag has no effect on value encoding
          wrappingTag = true;
      } break;

      //case EDAlternative::atgExplicit:
      default:
        wrappingTag = true;
      }

      if (tlv_prop.isDefinite())
        max_len = tlv_prop._valLen;
      rval += tDec->decode(use_enc + rval.nbytes, max_len - rval.nbytes);

      if (rval.isOk(relaxed_rule)) {
        if (edRes._alt->isOptional())
          markDecodedOptional(edRes._alt->getUId());
        //in case of wrapped alternative eat EOC's
        if (wrappingTag && tlv_prop.isIndefinite()) {
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

} //ber
} //asn1
} //eyeline

