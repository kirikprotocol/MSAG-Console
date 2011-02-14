/* ************************************************************************* *
 * Classes which implement ASN.1 type Tags
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TAGS_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_TAGS_DEFS__

#include <inttypes.h>
#include "eyeline/util/LWArray.hpp"
#include "core/buffers/FixedLengthString.hpp"

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

  static const unsigned _maxStringLen = sizeof("[UNI ]") + sizeof(ValueType)*3 + 1;

  typedef smsc::core::buffers::FixedLengthString<_maxStringLen> String_t;

  // --------- //
  TagClass_e  _tagClass;
  ValueType   _tagValue;

  ASTag(TagClass_e tag_class = tagUniversal, ValueType tag_val = 0)
    : _tagClass(tag_class), _tagValue(tag_val)
  { }
  ~ASTag()
  { }

  static const char * nmClass(TagClass_e class_id);
  //
  static String_t toString(const ASTag & use_tag);

  String_t toString(void) const { return toString(*this); }


  bool operator== (const ASTag & cmp_tag) const
  {
    return (_tagClass == cmp_tag._tagClass) && (_tagValue == cmp_tag._tagValue);
  }

  bool operator!= (const ASTag & cmp_tag) const
  {
    return (_tagClass != cmp_tag._tagClass) || (_tagValue != cmp_tag._tagValue);
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

typedef ASTag::String_t ASTagString;

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

extern ASTag  _tagANYTYPE;           //(-1) - RESERVED TAG
/* ************************************************************************* *
 * ASN type tagging environment
 * ************************************************************************* */

static const uint8_t _ASTaggingDFLT_SZ = 4;
typedef eyeline::util::LWArray_T<ASTag, uint8_t, _ASTaggingDFLT_SZ> ASTagsArray;

//ASN type complete tagging (vector of ASTags)
//NOTE: Overall number of tags is limited to 255
class ASTagging : public ASTagsArray {
public:
  enum Environment_e {
    tagsIMPLICIT = 0, //type is identified only by outermost tag
    tagsEXPLICIT      //type is identified by full set of tags
  };

protected:
  Environment_e _tagEnv;

  using ASTagsArray::prepend;
  using ASTagsArray::shiftLeft;
  using ASTagsArray::shiftRight;
  using ASTagsArray::operator=;
  using ASTagsArray::operator+=;
  using ASTagsArray::operator>>;
  using ASTagsArray::operator<<;

public:
  typedef ASTagsArray::size_type size_type;
  //
  ASTagging() //NOTE: set tagsEXPLICIT in order to allow conjoin() operation
    : ASTagsArray(), _tagEnv(tagsEXPLICIT) 
  { }
  //
  ASTagging(ASTag::TagClass_e tag_class, ASTag::ValueType use_tag,
            Environment_e use_env)
    : ASTagsArray(1), _tagEnv(use_env)
  {
    ASTagsArray::at(0) = ASTag(tag_class, use_tag);
  }
  //
  ASTagging(const ASTag & use_tag, Environment_e use_env)
    : ASTagsArray(1), _tagEnv(use_env)
  {
    ASTagsArray::at(0) = use_tag;
  }
  //Copying constructor
  ASTagging(const ASTagging & use_tags)
    : ASTagsArray(use_tags), _tagEnv(use_tags._tagEnv)
  { }

  //Conjoining constructors
  ASTagging(const ASTag & use_tag, Environment_e use_env,
            const ASTagging & inner_tags)
    : ASTagsArray(1), _tagEnv(use_env)
  {
    ASTagsArray::at(0) = use_tag;
    conjoin(inner_tags);
  }
  ASTagging(const ASTagging & outer_tags, const ASTagging & inner_tags)
    : ASTagsArray(outer_tags), _tagEnv(outer_tags._tagEnv)
  {
    conjoin(inner_tags);
  }

  ASTagging(Environment_e use_env, uint8_t num_tags, const ASTag * use_tag1, ... /* , const ASTag * use_tagN*/);
  //
  ~ASTagging()
  { }

  void init(const ASTag & use_tag, Environment_e use_env)
  {
    clear();
    append(use_tag);
    _tagEnv = use_env;
  }

  Environment_e getEnvironment(void) const { return _tagEnv; }
  void setEnvironment(Environment_e use_env) { _tagEnv = use_env; }

  bool isImplicit(void) const { return (_tagEnv == tagsIMPLICIT); }

  const ASTag & outerTag(void) const { return at(0); }
  const ASTag & innerTag(void) const { return atLast(); }

  //Compares taggings in accordance with ASN.1 canonical tags order.
  bool operator< (const ASTagging & cmp_tags) const
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

  //Applies this tagging to tagged type identified by given tag (X.690 cl. 8.14)
  //Composes resulting tagging according to own tagging environment
  //NOTE: throws if resulting number of tags exceeds possible limit
  void conjoin(const ASTag & add_tag) //throw(std::exception)
  {
    if (_tagEnv == tagsEXPLICIT)
      *this += add_tag;
  }
  //Applies this tagging to tagged type identified by given tagging (X.690 cl. 8.14)
  //Composes resulting tagging according to own tagging environment
  //NOTE: throws either if resulting number of tags exceeds possible limit.
  void conjoin(const ASTagging & add_tags) //throw(std::exception)
  {
    if (!add_tags.empty()) {
      if (_tagEnv == tagsEXPLICIT)
        *this += add_tags[0];
      //add rest of tags, which are considered as the part of referenced type content
      for (uint8_t i = 1; i < add_tags.size(); ++i)
        *this += add_tags[i];
    }
  }

  //Applies given tag to tagged type identified by this tagging (X.690 cl. 8.14)
  //according to given tagging environment.
  //NOTE: throws if resulting number of tags exceeds possible limit
  void applyTag(const ASTag & add_tag, Environment_e tag_env) //throw(std::exception)
  {
    if (tag_env == tagsEXPLICIT) {
      prepend(add_tag);
    } else {      //tagsIMPLICIT
      clear();
      at(0) = add_tag;
    }
    _tagEnv = tag_env;
  }
};

/* ************************************************************************* *
 * UNIVERSAL TYPES TAGGING
 * ************************************************************************* */
extern ASTagging  _tagsUNI0;             //0, IMPLICIT
/**/
extern ASTagging  _tagsBOOL;             //1, IMPLICIT
extern ASTagging  _tagsINTEGER;          //2, IMPLICIT
extern ASTagging  _tagsBITSTR;           //3, IMPLICIT
extern ASTagging  _tagsOCTSTR;           //4, IMPLICIT
extern ASTagging  _tagsNULL;             //5, IMPLICIT
extern ASTagging  _tagsObjectID;         //6, IMPLICIT
extern ASTagging  _tagsObjDescriptor;    //7, IMPLICIT
extern ASTagging  _tagsEXTERNAL;         //8, IMPLICIT
extern ASTagging  _tagsREAL;             //9, IMPLICIT
extern ASTagging  _tagsENUM;             //10, IMPLICIT
extern ASTagging  _tagsEmbeddedPDV;      //11, IMPLICIT
extern ASTagging  _tagsUTF8STR;          //12, IMPLICIT
extern ASTagging  _tagsRelativeOID;      //13, IMPLICIT

extern ASTagging  _tagsSEQOF;            //16, IMPLICIT
extern ASTagging  _tagsSETOF;            //17, IMPLICIT
extern ASTagging  _tagsNumericSTR;       //18, IMPLICIT
extern ASTagging  _tagsPrintableSTR;     //19, IMPLICIT
extern ASTagging  _tagsTeletexSTR;       //20, IMPLICIT
extern ASTagging  _tagsVideotexSTR;      //21, IMPLICIT
extern ASTagging  _tagsIA5STR;           //22, IMPLICIT
extern ASTagging  _tagsUTCTime;          //23, IMPLICIT
extern ASTagging  _tagsGeneralizedTime;  //24, IMPLICIT
extern ASTagging  _tagsGraphicSTR;       //25, IMPLICIT
extern ASTagging  _tagsVisibleSTR;       //26, IMPLICIT
extern ASTagging  _tagsGeneralSTR;       //27, IMPLICIT

extern ASTagging  _tagsCHARSTR;          //29, IMPLICIT
extern ASTagging  _tagsBMPSTR;           //30, IMPLICIT


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

