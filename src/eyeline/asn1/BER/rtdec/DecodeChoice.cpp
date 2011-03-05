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

//Sets tagged alternative of some type
void DecoderOfChoiceAC::setAlternative(uint16_t unique_idx, const ASTag & fld_tag, ASTagging::Environment_e fld_env)
  /*throw(std::exception)*/
{
  if (!isTagged()) //untagged CHOICE
    _altTags.addTagging(fld_tag, fld_env);
  _elDec->setAlternative(unique_idx, fld_tag, fld_env, EDAlternative::altOPTIONAL);
}

//Sets untagged alternative of ordinary type
void DecoderOfChoiceAC::setAlternative(uint16_t unique_idx, const ASTag & fld_tag)
  /*throw(std::exception)*/
{
  if (!isTagged()) //untagged CHOICE
    _altTags.addTagging(fld_tag, ASTagging::tagsIMPLICIT);
  _elDec->setAlternative(unique_idx, fld_tag, EDAlternative::altOPTIONAL);
}

//Sets untagged alternative of ANY/Opentype
void DecoderOfChoiceAC::setAlternative(uint16_t unique_idx)
  /* throw(std::exception)*/
{
  if (!isTagged()) //untagged CHOICE
    _altTags.addTagging(asn1::_uniTag().ANYTYPE, ASTagging::tagsIMPLICIT);
  _elDec->setAlternative(unique_idx, EDAlternative::altOPTIONAL);
}

//Sets untagged alternative of untagged CHOICE type
void DecoderOfChoiceAC::setAlternative(uint16_t unique_idx, const TaggingOptions & use_tag_opts)
  /*throw(std::exception)*/
{
  if (!isTagged()) //untagged CHOICE
    _altTags.addOptions(use_tag_opts);
  _elDec->setAlternative(unique_idx, use_tag_opts, EDAlternative::altOPTIONAL);
}

//Sets alternative for unknown extension additions entry
void DecoderOfChoiceAC::setUnkExtension(uint16_t unique_idx) /* throw(std::exception)*/
{
  if (!isTagged()) //untagged CHOICE
    _altTags.addTagging(asn1::_uniTag().UNI0, ASTagging::tagsIMPLICIT);
  _elDec->setUnkExtension(unique_idx);
}

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

