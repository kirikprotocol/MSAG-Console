#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <stdarg.h>
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace asn1 {

/* ************************************************************************* *
 * class ASTag implementation
 * ************************************************************************* */

/* ************************************************************************* *
 * class ASTagging implementation
 * ************************************************************************* */
ASTagging::ASTagging(uint8_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/)
  : _heapBuf(false), _numTags(num_tags), _tags(_stags)
{
  if (_numTags > _max_STACK_TAGS) {
    _heapBuf = true;
    _tags = new ASTag[_numTags];
  }

  _tags[0] = use_tag1;
  va_list  useTags;
  va_start(useTags, use_tag1);
  for (uint8_t i = 1; i < _numTags; ++i)
    _tags[i] = va_arg(useTags, ASTag);
  va_end(useTags);
}

} //asn1
} //eyeline

