#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class RCSTRValueDecoder implementation:
 * ************************************************************************* */

// -- ************************************* --
// -- ValueDecoderIface interface methods
// -- ************************************* --

DECResult RCSTRValueDecoder::decodeVAL(const TLVProperty * val_prop,
                    const uint8_t * use_enc, TSLength max_len,
                    TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                    bool relaxed_rule/* = false*/)
  /*throw(BERDecoderException)*/
{
  if (!val_prop)
    throw smsc::util::Exception("ber::RCSTRValueDecoder::decodeVal(): V-part properties isn't decoded");
  
  bool      strictDER = (!relaxed_rule && (use_rule == TSGroupBER::ruleDER));
  bool      strictCER = (!relaxed_rule && (use_rule == TSGroupBER::ruleCER));
  DECResult rval(DECResult::decBadEncoding);
  TSLength  valLen = val_prop->_valLen;

  if (!val_prop->_isConstructed) {
    if (val_prop->isIndefinite()) { //check DER/CER restrictions
      if (strictDER || strictCER)
        return rval;

      if (!valLen) {
        DECResult reoc = searchEOC(use_enc, max_len);
        if (!reoc.isOk())
          return reoc;
        valLen = reoc.nbytes;
      }
    }
    if (strictCER && (valLen > 1000))
      return rval;

    rval = _strConv->unpackFragment(use_enc, valLen);
    if (rval.nbytes < valLen)
      rval.status = DECResult::decBadEncoding;
    /**/
  } else { //constructed encoding, several fragments follow
    //NOTE: content octets [+ EOC] cann't be < 2 octets length ({Tag, 0} || EOC)
    if ((max_len < 2) || strictDER || (val_prop->isDefinite() && strictCER))
      return rval;

    if (val_prop->isIndefinite()) {
      //check for degenerate case: empty constructed encoding with indefinite LD
      if (checkEOC(use_enc, max_len))
        return strictCER ? rval : DECResult(DECResult::decOk);
      max_len -= 2; //exclude outermost EOC
    }

    TSLength  prevFragmSz = 0;
    rval.status = DECResult::decOk;

    while (rval.nbytes < max_len) {
      TLParser fragm;

      rval += fragm.decodeBOC(use_enc + rval.nbytes, max_len - rval.nbytes);
      if (!rval.isOk(relaxed_rule))
        return rval;
      //fragment must be primitive OCTET/BIT STRING value encoding
      if ((fragm._tag != _strConv->fragmentTag()) || fragm._isConstructed) {
        rval.status = DECResult::decBadEncoding;
        return rval;
      }
      //check CER restrictions on fragment size/LDForm
      if (strictCER && (fragm.isIndefinite() || (prevFragmSz && (prevFragmSz != 1000)))){
        rval.status = DECResult::decBadEncoding;
        return rval;
      }
      if (fragm.isIndefinite()) {
        DECResult reoc = searchEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
        if (!reoc.isOk())
          return reoc;
        fragm._valLen = reoc.nbytes;
      } else if (fragm._valLen > (max_len - rval.nbytes)) {
        rval.status = DECResult::decMoreInput;
        return rval;
      }
      if (!fragm._valLen) {
        rval.status = DECResult::decBadEncoding;
        return rval;
      }
      rval += _strConv->unpackFragment(use_enc + rval.nbytes, prevFragmSz = fragm._valLen);
      if (!rval.isOk(relaxed_rule))
        return rval;
    }
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

