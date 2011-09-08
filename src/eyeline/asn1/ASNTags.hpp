/* ************************************************************************* *
 * Classes which implement ASN.1 type Tags
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TAGS_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ABSTRACT_SYNTAX_TAGS_DEFS__

#include <inttypes.h>
#include "core/buffers/LWArrayT.hpp"
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

  explicit ASTag(TagClass_e tag_class = tagUniversal, ValueType tag_val = 0)
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
struct UniversalTags {
  ASTag  UNI0;             //0 - RESERVED TAG
  ASTag  BOOL;             //1                    
  ASTag  INTEGER;          //2                    
  ASTag  BITSTR;           //3                    
  ASTag  OCTSTR;           //4                    
  ASTag  NULLTYPE;         //5                    
  ASTag  ObjectID;         //6                    
  ASTag  ObjDescriptor;    //7                    
  ASTag  EXTERNAL;         //8                    
  ASTag  REAL;             //9                    
  ASTag  ENUM;             //10                   
  ASTag  EmbeddedPDV;      //11                   
  ASTag  UTF8STR;          //12                   
  ASTag  RelativeOID;      //13                   
                                                  
  ASTag  SEQOF;            //16                   
  ASTag  SETOF;            //17                   
  ASTag  NumericSTR;       //18                   
  ASTag  PrintableSTR;     //19                   
  ASTag  TeletexSTR;       //20                   
  ASTag  VideotexSTR;      //21                   
  ASTag  IA5STR;           //22                   
  ASTag  UTCTime;          //23                   
  ASTag  GeneralizedTime;  //24                   
  ASTag  GraphicSTR;       //25                   
  ASTag  VisibleSTR;       //26                   
  ASTag  GeneralSTR;       //27                   
                                                  
  ASTag  CHARSTR;          //29                   
  ASTag  BMPSTR;           //30                   
                                                  
  ASTag  ANYTYPE;           //(-1) - RESERVED TAG 

  UniversalTags();
  ~UniversalTags() { }
};

extern const UniversalTags & _uniTag(void);

/* ************************************************************************* *
 * ASN type tagging environment
 * ************************************************************************* */

static const uint8_t _ASTaggingDFLT_SZ = 4;
typedef smsc::core::buffers::LWArray_T<ASTag, uint8_t, _ASTaggingDFLT_SZ> ASTagsArray;

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
  //typedef ASTagsArray::size_type size_type;
  using ASTagsArray::size_type;

  //
  ASTagging() //NOTE: set tagsEXPLICIT in order to allow conjoin() operation
    : ASTagsArray(), _tagEnv(tagsEXPLICIT) 
  { }
  //
  ASTagging(ASTag::TagClass_e tag_class, ASTag::ValueType use_tag,
            Environment_e use_env)
    : ASTagsArray(1), _tagEnv(use_env)
  {
    append(ASTag(tag_class, use_tag));
  }
  //
  ASTagging(const ASTag & use_tag, Environment_e use_env)
    : ASTagsArray(1), _tagEnv(use_env)
  {
    append(use_tag);
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
    append(use_tag);
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
  bool operator< (const ASTagging & cmp_tags) const;
  //
  bool operator== (const ASTagging & cmp_tags) const;

  //Applies this tagging to tagged type identified by given tag (X.690 cl. 8.14)
  //Composes resulting tagging according to own tagging environment
  //NOTE: throws if resulting number of tags exceeds possible limit
  void conjoin(const ASTag & add_tag) /*throw(std::exception)*/
  {
    if (_tagEnv == tagsEXPLICIT)
      *this += add_tag;
  }
  //Applies this tagging to tagged type identified by given tagging (X.690 cl. 8.14)
  //Composes resulting tagging according to own tagging environment
  //NOTE: throws either if resulting number of tags exceeds possible limit.
  void conjoin(const ASTagging & add_tags) /*throw(std::exception)*/;

  //Applies given tag to tagged type identified by this tagging (X.690 cl. 8.14)
  //according to given tagging environment.
  //NOTE: throws if resulting number of tags exceeds possible limit
  void applyTag(const ASTag & add_tag, Environment_e tag_env) /*throw(std::exception)*/;
};

/* ************************************************************************* *
 * UNIVERSAL TYPES TAGGING
 * ************************************************************************* */
struct UniversalTagging {
  ASTagging  UNI0;             //0 - RESERVED TAG
  ASTagging  BOOL;             //1                    
  ASTagging  INTEGER;          //2                    
  ASTagging  BITSTR;           //3                    
  ASTagging  OCTSTR;           //4                    
  ASTagging  NULLTYPE;         //5                    
  ASTagging  ObjectID;         //6                    
  ASTagging  ObjDescriptor;    //7                    
  ASTagging  EXTERNAL;         //8                    
  ASTagging  REAL;             //9                    
  ASTagging  ENUM;             //10                   
  ASTagging  EmbeddedPDV;      //11                   
  ASTagging  UTF8STR;          //12                   
  ASTagging  RelativeOID;      //13                   
                                                  
  ASTagging  SEQOF;            //16                   
  ASTagging  SETOF;            //17                   
  ASTagging  NumericSTR;       //18                   
  ASTagging  PrintableSTR;     //19                   
  ASTagging  TeletexSTR;       //20                   
  ASTagging  VideotexSTR;      //21                   
  ASTagging  IA5STR;           //22                   
  ASTagging  UTCTime;          //23                   
  ASTagging  GeneralizedTime;  //24                   
  ASTagging  GraphicSTR;       //25                   
  ASTagging  VisibleSTR;       //26                   
  ASTagging  GeneralSTR;       //27                   
                                                  
  ASTagging  CHARSTR;          //29                   
  ASTagging  BMPSTR;           //30                   
                                                  
  ASTagging  ANYTYPE;           //(-1) - RESERVED TAG 

  UniversalTagging();
  ~UniversalTagging() { }
};

extern const UniversalTagging & _uniTagging(void);

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

