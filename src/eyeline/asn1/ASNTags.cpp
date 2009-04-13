#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <stdarg.h>
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace asn1 {

/* ************************************************************************* *
 * class ASTag implementation
 * ************************************************************************* */
bool ASTag::decode(const uint8_t * use_enc, uint16_t enc_len)
{
  bool  done = false;
  if (enc_len) {
    uint8_t l7b = use_enc[0];

    tagClass = static_cast<ASTag::TagClass_e>(l7b & 0xC0);

    if ((l7b &= 0x1F) == 0x1F) {  //long tag grater than 30
      unsigned i = 0;
      l7b = 0x80;
      while ((l7b >= 0x80) && (i < enc_len)
             && (i < ((sizeof(uint16_t)<<3)/7))) {
        l7b = use_enc[1 + i++];
        tagValue <<= 7;
        tagValue |= (uint16_t)(l7b & 0x7F);
      }
      if (l7b < 0x80)
        done = true;
    } else {            //short tag less than or equal to 30
      tagValue = (uint16_t)l7b;
      done = true;
    }
  }
  return done;
}

/* ************************************************************************* *
 * class ASTagging implementation
 * ************************************************************************* */
ASTagging::ASTagging(uint16_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/)
    : std::vector<ASTag>(num_tags)
{
  at(0) = use_tag1;
  va_list  useTags;
  va_start(useTags, use_tag1);
  for (uint16_t i = 1; i < num_tags; ++i)
    at(i) = va_arg(useTags, ASTag);
  va_end(useTags);
}

} //asn1
} //eyeline

