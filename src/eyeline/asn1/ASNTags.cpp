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
static const char * const _nmTagClass[4] = {"UNI", "APP", "CTX", "PRV" };

const char * ASTag::nmClass(ASTag::TagClass_e  class_id)
{
  return _nmTagClass[(class_id >> 6) & 0x03];
}

ASTagString ASTag::toString(const ASTag & use_tag)
{
  ASTagString rval;
  snprintf(rval.str, rval.capacity(), "[%s %u]",
           ASTag::nmClass(use_tag._tagClass), use_tag._tagValue);
  return rval;
}

/* ************************************************************************* *
 * class ASTagging implementation
 * ************************************************************************* */
ASTagging::ASTagging(Environment_e use_env,
                     uint8_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/)
  : ASTagsArray(num_tags), _tagEnv(use_env)
{
  ASTagsArray::_buf[0] = use_tag1;

  va_list  useTags;
  va_start(useTags, use_tag1);
  for (uint8_t i = 1; i < num_tags; ++i)
    ASTagsArray::_buf[i] = va_arg(useTags, ASTag);
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

ASTag  _tagANYTYPE(ASTag::tagUniversal, (ASTag::ValueType)(-1));
/* ************************************************************************* *
 * UNIVERSAL TYPES TAGGING
 * ************************************************************************* */
ASTagging  _tagsUNI0(_tagUNI0, ASTagging::tagsIMPLICIT);
ASTagging  _tagsBOOL(_tagBOOL, ASTagging::tagsIMPLICIT);
ASTagging  _tagsINTEGER(_tagINTEGER, ASTagging::tagsIMPLICIT);
ASTagging  _tagsBITSTR(_tagBITSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsOCTSTR(_tagOCTSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsNULL(_tagNULL, ASTagging::tagsIMPLICIT);
ASTagging  _tagsObjectID(_tagObjectID, ASTagging::tagsIMPLICIT);
ASTagging  _tagsObjDescriptor(_tagObjDescriptor, ASTagging::tagsIMPLICIT);
ASTagging  _tagsEXTERNAL(_tagEXTERNAL, ASTagging::tagsIMPLICIT);
ASTagging  _tagsREAL(_tagREAL, ASTagging::tagsIMPLICIT);
ASTagging  _tagsENUM(_tagENUM, ASTagging::tagsIMPLICIT);
ASTagging  _tagsEmbeddedPDV(_tagEmbeddedPDV, ASTagging::tagsIMPLICIT);
ASTagging  _tagsUTF8STR(_tagUTF8STR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsRelativeOID(_tagRelativeOID, ASTagging::tagsIMPLICIT);
                                  
ASTagging  _tagsSEQOF(_tagSEQOF, ASTagging::tagsIMPLICIT);
ASTagging  _tagsSETOF(_tagSETOF, ASTagging::tagsIMPLICIT);
ASTagging  _tagsNumericSTR(_tagNumericSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsPrintableSTR(_tagPrintableSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsTeletexSTR(_tagTeletexSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsVideotexSTR(_tagVideotexSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsIA5STR(_tagIA5STR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsUTCTime(_tagUTCTime, ASTagging::tagsIMPLICIT);
ASTagging  _tagsGeneralizedTime(_tagGeneralizedTime, ASTagging::tagsIMPLICIT);
ASTagging  _tagsGraphicSTR(_tagGraphicSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsVisibleSTR(_tagVisibleSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsGeneralSTR(_tagGeneralSTR, ASTagging::tagsIMPLICIT);
                                  
ASTagging  _tagsCHARSTR(_tagCHARSTR, ASTagging::tagsIMPLICIT);
ASTagging  _tagsBMPSTR(_tagBMPSTR, ASTagging::tagsIMPLICIT);
} //asn1
} //eyeline

