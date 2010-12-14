#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/TaggingDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class TaggingDecoder implementation:
 * ************************************************************************* */

//Decodes all TL-pairs of TLV layout
DECResult TaggingDecoder::decodeBOC(const uint8_t * use_enc, TSLength max_len,
                                      bool relaxed_rule/* = false*/)
  /* throw(BERDecoderException) */
{
  DECResult rval(DECResult::decOk);
  if (!_effTags)
    return rval;

  //traverse 'TL'-pairs from first to last
  if (_outerTL)
    _tlws[0] = *_outerTL;
  for (ASTagging::size_type i = (_outerTL ? 1 : 0); i < _effTags->size(); ++i) {
    rval += _tlws[i].decodeTOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    if (!rval.isOk())
      break;

    if (_tlws[i]._tag != (*_effTags)[i]) {
      rval.status = DECResult::decBadEncoding;
      break;
      /* TODO: log or throw
         TLVParserException(rval, "TaggingDecoder: tag[%u] = %s instead of %s",
                                i, _tlws[i]._tag.toString().c_str(),
                                (*_effTags)[i].toString().c_str());
      */
    }
    rval += _tlws[i].decodeLOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    if (!rval.isOk(relaxed_rule))
      break;
  }
  return rval;
}

//Decodes all 'end-of-content' octets of TLV layout
DECResult TaggingDecoder::decodeEOC(const uint8_t * use_enc, TSLength max_len) const
{
  DECResult rval(DECResult::decOk);
  if (_effTags->empty())
    return rval;

  //NOTE: traverse 'TL'-pairs from last to first even if max_len == 0
  //in order to detect missed EOCs for indefinite LDs
  ASTagging::size_type i = _effTags->size();
  do {
    rval += _tlws[--i].decodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  } while (i && rval.isOk());

  return rval;
}

} //ber
} //asn1
} //eyeline

