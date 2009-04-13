/* ************************************************************************* *
 * Classes which implement ASN.1 type Tags
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TAGS_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TAGS_DEFS__

#include <inttypes.h>
#include <vector>

namespace eyeline {
namespace asn1 {

struct ASTag {
  enum TagClass_e {
    tagUniversal = 0x00,
    tagApplication = 0x40,
    tagContextSpecific = 0x80,
    tagPrivate = 0xB0
  };

  TagClass_e  tagClass;
  uint16_t    tagValue; //holds 16k (2^14-1) values

  ASTag(TagClass_e tag_class = tagUniversal, uint16_t tag_val = 0)
    : tagClass(tag_class), tagValue(tag_val)
  { }

  //NOTE: in case of invalid encoding, ASTag is set to UNIVERSAL::0
  ASTag(const uint8_t * use_enc, uint16_t enc_len)
  {
    if (!decode(use_enc, enc_len)) {
      tagClass = tagUniversal; tagValue = 0;
    }
  }

  //Returns false in case of invalid tag encoding
  bool decode(const uint8_t * use_enc, uint16_t enc_len);

  bool operator== (const ASTag & cmp_tag) const
  {
    return (tagClass == cmp_tag.tagClass) && (tagValue == cmp_tag.tagValue);
  }

  //Compares tags in accordance with ASN.1 canonical tags order as stated
  //in X.680 cl.8.6:
  //  The canonical order for tags is based on the outermost tag of each type
  //  and is defined as follows:
  //  a) those elements or alternatives with universal class tags shall appear
  //    first, followed by those with application class tags, followed by those
  //    with context-specific tags, followed by those with private class tags;
  //  b) within each class of tags, the elements or alternatives shall appear
  //    in ascending order of their tag numbers.
  bool operator< (const ASTag & cmp_tag) const
  {
    if (tagClass == cmp_tag.tagClass)
        return (tagValue < cmp_tag.tagValue);
    return (tagClass < cmp_tag.tagClass);
  }
};

//Abstract type complete tagging (all tags goes to encoding in case of
//EXPLICIT tagging environment).
class ASTagging : std::vector<ASTag> {
public:
  //just a single tag, by default: [UNIVERSAL 0]
  ASTagging(ASTag::TagClass_e tag_class = ASTag::tagUniversal, uint16_t tag_val = 0)
    : std::vector<ASTag>(1)
  {
    at(0) = ASTag(tag_class, tag_val);
  }
  ASTagging(const ASTag & use_tag) //just a single tag
    : std::vector<ASTag>(1)
  {
    at(0) = use_tag;
  }
  ASTagging(const ASTagging & use_tags)
    : std::vector<ASTag>(use_tags)
  { }
  ASTagging(uint16_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/);
  //
  ~ASTagging()
  { }


  uint16_t numTags(void) const
  {
    return (uint16_t)(size());
  }
  //tag_idx = 0 - outermost tag, in most cases this is just a type tag
  //NOTE: It's a caller responsibility to ensure tag_idx isn't out of range
  const ASTag & Tag(uint16_t tag_idx = 0) const
  {
    return tag_idx < numTags() ? at(tag_idx) : at(0);
  }

  //Compares taggings in accordance with ASN.1 canonical tags order.
  bool operator< (const ASTagging & cmp_tags) const
  {
    return at(0) < cmp_tags.Tag(0);
  }
};


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

