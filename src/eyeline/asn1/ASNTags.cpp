#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <stdarg.h>
#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace asn1 {

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

/* ************************************************************************* *
 * NIVERSAL CLASS TAG NUMBERS
 * ************************************************************************* */

ASTagging  _tagBOOL(ASTag::tagUniversal, 1);
ASTagging  _tagINTEGER(ASTag::tagUniversal, 2);
ASTagging  _tagBITSTR(ASTag::tagUniversal, 3);
ASTagging  _tagOCTSTR(ASTag::tagUniversal, 4);
ASTagging  _tagNULL(ASTag::tagUniversal, 5);
ASTagging  _tagObjectID(ASTag::tagUniversal, 6);
ASTagging  _tagObjDescriptor(ASTag::tagUniversal, 7);
ASTagging  _tagEXTERNAL(ASTag::tagUniversal, 8);
ASTagging  _tagREAL(ASTag::tagUniversal, 9);
ASTagging  _tagENUM(ASTag::tagUniversal, 10);
ASTagging  _tagEmbeddedPDV(ASTag::tagUniversal, 11);
ASTagging  _tagUTF8STR(ASTag::tagUniversal, 12);
ASTagging  _tagRelativeID(ASTag::tagUniversal, 13);

ASTagging  _tagSEQOF(ASTag::tagUniversal, 16);
ASTagging  _tagSETOF(ASTag::tagUniversal, 17);
ASTagging  _tagNumericSTR(ASTag::tagUniversal, 18);
ASTagging  _tagPrintableSTR(ASTag::tagUniversal, 19);
ASTagging  _tagTeletexSTR(ASTag::tagUniversal, 20);
ASTagging  _tagVideotexSTR(ASTag::tagUniversal, 21);
ASTagging  _tagIA5STR(ASTag::tagUniversal, 22);
ASTagging  _tagUTCTime(ASTag::tagUniversal, 23);
ASTagging  _tagGeneralizedTime(ASTag::tagUniversal, 24);
ASTagging  _tagGraphicSTR(ASTag::tagUniversal, 25);
ASTagging  _tagVisibleSTR(ASTag::tagUniversal, 26);
ASTagging  _tagGeneralSTR(ASTag::tagUniversal, 27);

ASTagging  _tagCHARSTR(ASTag::tagUniversal, 29);
ASTagging  _tagBMPSTR(ASTag::tagUniversal, 30);


} //asn1
} //eyeline

