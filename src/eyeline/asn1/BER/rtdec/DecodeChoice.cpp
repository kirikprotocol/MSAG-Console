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

// ------------------------------------------------------------------------
// -- DecoderOfChoiceAC interface methods
// ------------------------------------------------------------------------

//NOTE: in case of Untagged CHOICE/ANY/OpenType the identification tag is a
//      part of value encoding.
DECResult DecoderOfChoiceAC::decodeVAL(const TLParser & tlv_prop,
                            const uint8_t * use_enc, TSLength max_len,
                            TSGroupBER::Rule_e use_rule/* = TSGroupBER::ruleBER*/,
                            bool relaxed_rule/* = false*/)
  /*throw(std::exception)*/
{
  _elDec->reset();
  if (!isTagged()) //identification tag is a part of value encoding
    return decodeElement(tlv_prop, use_enc, max_len, relaxed_rule);

  if (tlv_prop.isDefinite())
    max_len = tlv_prop._valLen;

  TLParser  vtl;
  DECResult rval = vtl.decodeBOC(use_enc, max_len);

  if (rval.isOk(_relaxedRule))
    rval += decodeElement(vtl, use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
  return rval;
}

} //ber
} //asn1
} //eyeline

