#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

bool checkEOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/
{
  return (max_len > 1)  ? (!use_enc[0] && !use_enc[1]) : false;
}

//Searches 'content octets' of primitive encoding for EOC octets (two zeroes)
//Returns number of bytes preceeding EOC if it's found
DECResult searchEOC(const uint8_t * use_enc, TSLength max_len)
{
  for (TSLength i = 0; i < max_len; ++i) {
    if (!use_enc[i] && ((i + 1) < max_len) && !use_enc[i+1])
      return DECResult(DECResult::decOk, i);
  }
  return DECResult(DECResult::decMoreInput);
}

//Searches 'content octets' of constructed encoding for outermost EOC octets (two zeroes)
//Returns number of bytes preceeding outermost EOC if it's found
DECResult searchEOCconstructed(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule/* = true */)
{
  DECResult rval(DECResult::decOk);
  while ((rval.nbytes < max_len) && rval.isOk(relaxed_rule))
    rval += skipTLV(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
  return rval;
}

//Returns number of bytes TLV occupies.
DECResult skipTLV(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule/* = true */)
{
  TLParser  tlProp;
  DECResult rval(DECResult::decOk);

  tlProp.decodeBOC(use_enc, max_len);
  if (!rval.isOk(relaxed_rule))
    return rval;

  if (tlProp.isDefinite()) {
    if ((rval.nbytes + tlProp._valLen) < max_len)
      rval.status = DECResult::decMoreInput;
    else
      rval.nbytes += tlProp._valLen;
    return rval;
  }
  //indefinite LDForm_e, search for EOCs
  if (!tlProp._isConstructed)
    rval += searchEOCconstructed(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
  else
    rval += searchEOC(use_enc + rval.nbytes, max_len - rval.nbytes);

  if (rval.isOk(relaxed_rule))
    rval.nbytes += 2;   //count EOCs
  return rval;
}

/* ************************************************************************* *
 * Class TaggingDecoder implementation:
 * ************************************************************************* */

//Decodes all TL-pairs of TLV layout
DECResult TaggingDecoder::decodeBOC(const uint8_t * use_enc, TSLength max_len,
                                      bool relaxed_rule/* = false*/)
  /* throw(BERDecoderException) */
{
  if (!max_len)
    return DECResult(_effTags ? DECResult::decMoreInput : DECResult::decOk);

  DECResult rval(DECResult::decOk);
  if (!_effTags)
    return rval;

  //traverse 'TL'-pairs from first to last
  for (ASTagging::size_type i = (_outerTL ? 1 : 0); i < _effTags->size(); ++i) {
    rval += _tlws[i].decodeTOC(use_enc - rval.nbytes, max_len + rval.nbytes);
    if (!rval.isOk())
      break;
    if (_tlws[i]._tag != (*_effTags)[i]) {
      rval.status = DECResult::decBadEncoding;
      break;
      /* TODO:
        throw TLVParserException(rval, "TaggingDecoder: tag[%u] = %s instead of %s",
                                i, _tlws[i]._tag.toString().c_str(),
                                (*_effTags)[i].toString().c_str());
      */
    }
    rval += _tlws[i].decodeLOC(use_enc - rval.nbytes, max_len + rval.nbytes);
    if (!rval.isOk(relaxed_rule))
      break;
  }
  return rval;
}

//Decodes all 'end-of-content' octets of TLV layout
DECResult TaggingDecoder::decodeEOC(const uint8_t * use_enc, TSLength max_len) const
{
  DECResult rval(DECResult::decOk);
  if (!_effTags->size())
    return rval;

  //traverse 'TL'-pairs from last to first
  for (ASTagging::size_type i = _effTags->size() - 1; ((i >= (_outerTL ? 1 : 0)) && rval.isOk()); --i) {
    rval += _tlws[i].decodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}

/* ************************************************************************* *
 * Class TypeDecoderAC implementation:
 * ************************************************************************* */
DECResult TypeDecoderAC::decode(const uint8_t * use_enc, TSLength max_len)
  /*throw(BERDecoderException)*/
{
  DECResult rval(DECResult::decOk);

  _tagDec.init(refreshTagging());
  rval += _tagDec.decodeBOC(use_enc, max_len, _relaxedRule); //throws
  if (rval.isOk(_relaxedRule)) {
    rval += _valDec->decodeVAL(_tagDec.getVProperties(), use_enc + rval.nbytes, max_len - rval.nbytes,
                             getVALRule(), _relaxedRule); //throws
    if (rval.isOk(_relaxedRule))
      rval += _tagDec.decodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

