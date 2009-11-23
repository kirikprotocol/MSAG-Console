/* ************************************************************************* *
 * Classes which implement ASN.1 type Tags
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TAGS_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TAGS_DEFS__

#include <inttypes.h>
#include "eyeline/util/LWArray.hpp"

namespace eyeline {
namespace asn1 {

struct ASTag {
  enum TagClass_e {
    tagUniversal = 0x00,
    tagApplication = 0x40,
    tagContextSpecific = 0x80,
    tagPrivate = 0xB0
  };
  typedef uint16_t ValueType;

  TagClass_e  _tagClass;
  ValueType   _tagValue;

  ASTag(TagClass_e tag_class = tagUniversal, ValueType tag_val = 0)
    : _tagClass(tag_class), _tagValue(tag_val)
  { }

  bool operator== (const ASTag & cmp_tag) const
  {
    return (_tagClass == cmp_tag._tagClass) && (_tagValue == cmp_tag._tagValue);
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
    if (_tagClass == cmp_tag._tagClass)
        return (_tagValue < cmp_tag._tagValue);
    return (_tagClass < cmp_tag._tagClass);
  }
};

/* ************************************************************************* *
 * UNIVERSAL CLASS TAG NUMBERS
 * ************************************************************************* */
extern ASTag  _tagUNI0;             //0 - RESERVED TAG
/**/
extern ASTag  _tagBOOL;             //1
extern ASTag  _tagINTEGER;          //2
extern ASTag  _tagBITSTR;           //3
extern ASTag  _tagOCTSTR;           //4
extern ASTag  _tagNULL;             //5
extern ASTag  _tagObjectID;         //6
extern ASTag  _tagObjDescriptor;    //7
extern ASTag  _tagEXTERNAL;         //8
extern ASTag  _tagREAL;             //9
extern ASTag  _tagENUM;             //10
extern ASTag  _tagEmbeddedPDV;      //11
extern ASTag  _tagUTF8STR;          //12
extern ASTag  _tagRelativeOID;      //13

extern ASTag  _tagSEQOF;            //16
extern ASTag  _tagSETOF;            //17
extern ASTag  _tagNumericSTR;       //18
extern ASTag  _tagPrintableSTR;     //19
extern ASTag  _tagTeletexSTR;       //20
extern ASTag  _tagVideotexSTR;      //21
extern ASTag  _tagIA5STR;           //22
extern ASTag  _tagUTCTime;          //23
extern ASTag  _tagGeneralizedTime;  //24
extern ASTag  _tagGraphicSTR;       //25
extern ASTag  _tagVisibleSTR;       //26
extern ASTag  _tagGeneralSTR;       //27

extern ASTag  _tagCHARSTR;          //29
extern ASTag  _tagBMPSTR;           //30

/* ************************************************************************* *
 * ASN type tagging environment
 * ************************************************************************* */

using eyeline::util::LWArray_T;
static const uint8_t _ASTaggingDFLT_SZ = 4;

//ASN type complete tagging (vector of ASTags)
//NOTE: Overall number of tags is limited to 255
class ASTagging : public LWArray_T<ASTag, uint8_t, _ASTaggingDFLT_SZ> {
protected:
  using LWArray_T<ASTag, uint8_t, _ASTaggingDFLT_SZ>::append;
  using LWArray_T<ASTag, uint8_t, _ASTaggingDFLT_SZ>::operator+=;

public:
  enum Environment_e {
    tagsEXPLICIT = 0,  //type is identified by full set of tags
    tagsIMPLICIT       //type is identified only by outermost tag
  };

protected:
  Environment_e _tagEnv;

  ASTag & innerTag(void) { return at(size() - 1); }

public:
  //just a single tag, by default: [UNIVERSAL 0] primitive
  ASTagging(ASTag::TagClass_e tag_class = ASTag::tagUniversal, ASTag::ValueType tag_val = 0,
            Environment_e use_env = tagsEXPLICIT)
    : LWArray_T<ASTag, uint8_t, 4>(1), _tagEnv(use_env)
  {
    LWArray_T<ASTag, uint8_t, 4>::_buf[0] = ASTag(tag_class, tag_val);
  }
  //
  ASTagging(const ASTag & use_tag, Environment_e use_env = tagsEXPLICIT)
    : LWArray_T<ASTag, uint8_t, 4>(1), _tagEnv(use_env)
  {
    LWArray_T<ASTag, uint8_t, 4>::_buf[0] = use_tag;
  }
  //
  ASTagging(const ASTagging & use_tags)
    : LWArray_T<ASTag, uint8_t, 4>(use_tags), _tagEnv(use_tags._tagEnv)
  { }

  //Conjoining constructor
  ASTagging(const ASTagging & outer_tags, const ASTagging & inner_tags)
    : LWArray_T<ASTag, uint8_t, 4>(outer_tags), _tagEnv(outer_tags._tagEnv)
  {
    conjoin(inner_tags);
  }


  ASTagging(uint8_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/);
  //
  ~ASTagging()
  { }

  Environment_e getEnvironment(void) const { return _tagEnv; }
  void setEnvironment(Environment_e use_env) { _tagEnv = use_env; }

  const ASTag & outerTag(void) const { return at(0); }
  const ASTag & innerTag(void) const { return at(size() - 1); }

  //Compares taggings in accordance with ASN.1 canonical tags order.
  bool operator< (const ASTagging & cmp_tags) const
  {
    uint8_t num2cmp = (size() <=  cmp_tags.size()) ? size() : cmp_tags.size();

    for (uint8_t i = 0; i < num2cmp; ++i) {
      if ((*this)[i] < cmp_tags[i])
        return true;
    }
    //all 'num2cmp' tags are equal, the shorter tagging is smaller one
    return (size() < cmp_tags.size());
  }

  bool operator== (const ASTagging & cmp_tags) const
  {
    if (size() !=  cmp_tags.size())
      return false;

    for (uint8_t i = 0; i < size(); ++i) {
      if (!((*this)[i] == cmp_tags[i]))
        return false;
    }
    return true;
  }

  //Applies this tagging to tagging of tagged type (X.690 cl. 8.14)
  //Composes resulting tagging according to environment
  void conjoin(const ASTag & add_tag)
  {
    if (_tagEnv == tagsEXPLICIT)
      *this += add_tag;
  }
  //NOTE: add_tags shouldn't be empty !
  void conjoin(const ASTagging & add_tags)
  {
    if (_tagEnv == tagsEXPLICIT)
      *this += add_tags[0];
    //add tags, which are the part of referenced type content
    for (uint8_t i = 1; i < add_tags.size(); ++i)
      *this += add_tags[i];
  }
};


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

