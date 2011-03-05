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
                     uint8_t num_tags, const ASTag * use_tag1, ... /* , const ASTag * use_tagN*/)
  : ASTagsArray(num_tags), _tagEnv(use_env)
{
  ASTagsArray::at(0) = *use_tag1;

  va_list  useTags;
  va_start(useTags, use_tag1);
  for (uint8_t i = 1; i < num_tags; ++i)
    ASTagsArray::at(i) = *va_arg(useTags, const ASTag*);
  va_end(useTags);
}

//Compares taggings in accordance with ASN.1 canonical tags order.
bool ASTagging::operator< (const ASTagging & cmp_tags) const
{
  uint8_t num2cmp = (size() <=  cmp_tags.size()) ? size() : cmp_tags.size();

  for (uint8_t i = 0; i < num2cmp; ++i) {
    if ((*this)[i] < cmp_tags[i])
      return true;
  }
  //all 'num2cmp' tags are equal, the shorter tagging is smaller one
  //if all tags are equal, compare environment
  return (size() == cmp_tags.size()) ? (_tagEnv < cmp_tags._tagEnv)
                                    : (size() < cmp_tags.size());
}

bool ASTagging::operator== (const ASTagging & cmp_tags) const
{
  if (size() !=  cmp_tags.size())
    return false;

  for (uint8_t i = 0; i < size(); ++i) {
    if (!((*this)[i] == cmp_tags[i]))
      return false;
  }
  return true;
}

void ASTagging::conjoin(const ASTagging & add_tags) /*throw(std::exception)*/
{
  if (!add_tags.empty()) {
    if (_tagEnv == tagsEXPLICIT)
      *this += add_tags[0];
    //add rest of tags, which are considered as the part of referenced type content
    for (uint8_t i = 1; i < add_tags.size(); ++i)
      *this += add_tags[i];
  }
}

void ASTagging::applyTag(const ASTag & add_tag, Environment_e tag_env)
  /*throw(std::exception)*/
{
  if (tag_env == tagsEXPLICIT) {
    prepend(add_tag);
  } else {      //tagsIMPLICIT
    clear();
    append(add_tag);
  }
  _tagEnv = tag_env;
}

/* ************************************************************************* *
 * UNIVERSAL CLASS TAG NUMBERS
 * ************************************************************************* */
UniversalTags::UniversalTags()
  : UNI0(ASTag::tagUniversal, 0), BOOL(ASTag::tagUniversal, 1)
  , INTEGER(ASTag::tagUniversal, 2), BITSTR(ASTag::tagUniversal, 3)
  , OCTSTR(ASTag::tagUniversal, 4), NULLTYPE(ASTag::tagUniversal, 5)
  , ObjectID(ASTag::tagUniversal, 6), ObjDescriptor(ASTag::tagUniversal, 7)
  , EXTERNAL(ASTag::tagUniversal, 8), REAL(ASTag::tagUniversal, 9)
  , ENUM(ASTag::tagUniversal, 10), EmbeddedPDV(ASTag::tagUniversal, 11)
  , UTF8STR(ASTag::tagUniversal, 12), RelativeOID(ASTag::tagUniversal, 13)
  , SEQOF(ASTag::tagUniversal, 16), SETOF(ASTag::tagUniversal, 17)
  , NumericSTR(ASTag::tagUniversal, 18), PrintableSTR(ASTag::tagUniversal, 19)
  , TeletexSTR(ASTag::tagUniversal, 20), VideotexSTR(ASTag::tagUniversal, 21)
  , IA5STR(ASTag::tagUniversal, 22), UTCTime(ASTag::tagUniversal, 23)
  , GeneralizedTime(ASTag::tagUniversal, 24), GraphicSTR(ASTag::tagUniversal, 25)
  , VisibleSTR(ASTag::tagUniversal, 26), GeneralSTR(ASTag::tagUniversal, 27)
  , CHARSTR(ASTag::tagUniversal, 29), BMPSTR(ASTag::tagUniversal, 30)
  , ANYTYPE(ASTag::tagUniversal, (ASTag::ValueType)(-1))
{ }

const UniversalTags & _uniTag(void)
{
  static UniversalTags  _tags;
  return _tags;
}

/* ************************************************************************* *
 * UNIVERSAL TYPES TAGGING
 * ************************************************************************* */
UniversalTagging::UniversalTagging()
  : UNI0(ASTag::tagUniversal, 0, ASTagging::tagsIMPLICIT)
  , BOOL(ASTag::tagUniversal, 1, ASTagging::tagsIMPLICIT)
  , INTEGER(ASTag::tagUniversal, 2, ASTagging::tagsIMPLICIT)
  , BITSTR(ASTag::tagUniversal, 3, ASTagging::tagsIMPLICIT)
  , OCTSTR(ASTag::tagUniversal, 4, ASTagging::tagsIMPLICIT)
  , NULLTYPE(ASTag::tagUniversal, 5, ASTagging::tagsIMPLICIT)
  , ObjectID(ASTag::tagUniversal, 6, ASTagging::tagsIMPLICIT)
  , ObjDescriptor(ASTag::tagUniversal, 7, ASTagging::tagsIMPLICIT)
  , EXTERNAL(ASTag::tagUniversal, 8, ASTagging::tagsIMPLICIT)
  , REAL(ASTag::tagUniversal, 9, ASTagging::tagsIMPLICIT)
  , ENUM(ASTag::tagUniversal, 10, ASTagging::tagsIMPLICIT)
  , EmbeddedPDV(ASTag::tagUniversal, 11, ASTagging::tagsIMPLICIT)
  , UTF8STR(ASTag::tagUniversal, 12, ASTagging::tagsIMPLICIT)
  , RelativeOID(ASTag::tagUniversal, 13, ASTagging::tagsIMPLICIT)
  , SEQOF(ASTag::tagUniversal, 16, ASTagging::tagsIMPLICIT)
  , SETOF(ASTag::tagUniversal, 17, ASTagging::tagsIMPLICIT)
  , NumericSTR(ASTag::tagUniversal, 18, ASTagging::tagsIMPLICIT)
  , PrintableSTR(ASTag::tagUniversal, 19, ASTagging::tagsIMPLICIT)
  , TeletexSTR(ASTag::tagUniversal, 20, ASTagging::tagsIMPLICIT)
  , VideotexSTR(ASTag::tagUniversal, 21, ASTagging::tagsIMPLICIT)
  , IA5STR(ASTag::tagUniversal, 22, ASTagging::tagsIMPLICIT)
  , UTCTime(ASTag::tagUniversal, 23, ASTagging::tagsIMPLICIT)
  , GeneralizedTime(ASTag::tagUniversal, 24, ASTagging::tagsIMPLICIT)
  , GraphicSTR(ASTag::tagUniversal, 25, ASTagging::tagsIMPLICIT)
  , VisibleSTR(ASTag::tagUniversal, 26, ASTagging::tagsIMPLICIT)
  , GeneralSTR(ASTag::tagUniversal, 27, ASTagging::tagsIMPLICIT)
  , CHARSTR(ASTag::tagUniversal, 29, ASTagging::tagsIMPLICIT)
  , BMPSTR(ASTag::tagUniversal, 30, ASTagging::tagsIMPLICIT)
  , ANYTYPE(ASTag::tagUniversal, (ASTag::ValueType)(-1), ASTagging::tagsIMPLICIT)
{ }

const UniversalTagging & _uniTagging(void)
{
  static UniversalTagging  _taggings;
  return _taggings;
}

} //asn1
} //eyeline

