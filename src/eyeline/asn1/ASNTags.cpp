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
  : LWArray_T<ASTag, uint8_t, 4>(num_tags)
{
  LWArray_T<ASTag, uint8_t, 4>::_buf[0] = use_tag1;

  va_list  useTags;
  va_start(useTags, use_tag1);
  for (uint8_t i = 1; i < num_tags; ++i)
    LWArray_T<ASTag, uint8_t, 4>::_buf[i] = va_arg(useTags, ASTag);
  va_end(useTags);
}

/* ************************************************************************* *
 * UNIVERSAL CLASS TAG NUMBERS
 * ************************************************************************* */
ASTag  _tagUNI0(ASTag::tagUniversal, 0);
ASTag  _tagBOOL(ASTag::tagUniversal, 1);
ASTag  _tagINTEGER(ASTag::tagUniversal, 2);
ASTag  _tagBITSTR(ASTag::tagUniversal, 3);
ASTag  _tagOCTSTR(ASTag::tagUniversal, 4);
ASTag  _tagNULL(ASTag::tagUniversal, 5);
ASTag  _tagObjectID(ASTag::tagUniversal, 6);
ASTag  _tagObjDescriptor(ASTag::tagUniversal, 7);
ASTag  _tagEXTERNAL(ASTag::tagUniversal, 8);
ASTag  _tagREAL(ASTag::tagUniversal, 9);
ASTag  _tagENUM(ASTag::tagUniversal, 10);
ASTag  _tagEmbeddedPDV(ASTag::tagUniversal, 11);
ASTag  _tagUTF8STR(ASTag::tagUniversal, 12);
ASTag  _tagRelativeOID(ASTag::tagUniversal, 13);

ASTag  _tagSEQOF(ASTag::tagUniversal, 16);
ASTag  _tagSETOF(ASTag::tagUniversal, 17);
ASTag  _tagNumericSTR(ASTag::tagUniversal, 18);
ASTag  _tagPrintableSTR(ASTag::tagUniversal, 19);
ASTag  _tagTeletexSTR(ASTag::tagUniversal, 20);
ASTag  _tagVideotexSTR(ASTag::tagUniversal, 21);
ASTag  _tagIA5STR(ASTag::tagUniversal, 22);
ASTag  _tagUTCTime(ASTag::tagUniversal, 23);
ASTag  _tagGeneralizedTime(ASTag::tagUniversal, 24);
ASTag  _tagGraphicSTR(ASTag::tagUniversal, 25);
ASTag  _tagVisibleSTR(ASTag::tagUniversal, 26);
ASTag  _tagGeneralSTR(ASTag::tagUniversal, 27);

ASTag  _tagCHARSTR(ASTag::tagUniversal, 29);
ASTag  _tagBMPSTR(ASTag::tagUniversal, 30);


} //asn1
} //eyeline

