/* ************************************************************************* *
 * Classes which implement ASN.1 type Tags
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TAGS_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TAGS_DEFS__

#include <inttypes.h>
#include "util/Exception.hpp"

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
  bool        _isConstructed;

  ASTag(TagClass_e tag_class = tagUniversal, ValueType tag_val = 0, bool _isConstructed = false)
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

//Abstract type complete tagging (vector of ASTags)
//NOTE: all tags goes to BER encoding in case of EXPLICIT tagging environment.
class ASTagging  {
protected:
  static const uint8_t _max_STACK_TAGS = 4;
  bool    _heapBuf;
  uint8_t _numTags;
  ASTag   _stags[_max_STACK_TAGS];  //Most of ASN.1 types have no more than 2 tags
  ASTag * _tags;

public:
  //just a single tag, by default: [UNIVERSAL 0]
  ASTagging(ASTag::TagClass_e tag_class = ASTag::tagUniversal,
            ASTag::ValueType tag_val = 0, bool tag_constructed = false)
    : _heapBuf(false), _numTags(1), _tags(_stags)
  {
    _stags[0] = ASTag(tag_class, tag_val, tag_constructed);
  }
  //
  ASTagging(const ASTag & use_tag) //just a single tag
    : _heapBuf(false), _numTags(1), _tags(_stags)
  {
    _stags[0] = use_tag;
  }
  //
  ASTagging(const ASTagging & use_tags)
    : _heapBuf(use_tags._heapBuf), _numTags(use_tags._numTags), _tags(_stags)
  {
    if (_heapBuf)
      _tags = new ASTag[_numTags];
    //copy tags
    for (uint8_t i = 0; i < _numTags; ++i)
      _tags[i] = use_tags._tags[i];
  }

  ASTagging(uint8_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/);
  //
  ~ASTagging()
  {
    if (_heapBuf)
      delete [] _tags;
  }


  uint8_t size(void) const { return _numTags; }

  const ASTag * begin(void) const { return _tags; }

  //tag_idx = 0 - outermost tag, in most cases this is just a type tag
  //NOTE: It's a caller responsibility to ensure tag_idx isn't out of range

  const ASTag & tagN(uint8_t tag_idx) const //throw std::exception
  {
    if (tag_idx >= _numTags)
      throw smsc::util::Exception("tag index=%u is out of range=%u",
                                  (unsigned)tag_idx, (unsigned)_numTags);
    return _tags[tag_idx];
  }
  // 
  const ASTag & operator[] (uint8_t tag_idx) const //throw std::exception
  {
    return tagN(tag_idx);
  }

  //Compares taggings in accordance with ASN.1 canonical tags order.
  bool operator< (const ASTagging & cmp_tags) const
  {
    uint8_t num2cmp = (_numTags <=  cmp_tags._numTags) ? _numTags : cmp_tags._numTags;

    for (uint8_t i = 0; i < num2cmp; ++i) {
      if (_tags[i] < cmp_tags._tags[i])
        return true;
    }
    //all 'num2cmp' tags are equal, the shorter tagging is smaller one
    return (_numTags < cmp_tags._numTags);
  }

  bool operator== (const ASTagging & cmp_tags) const
  {
    if (_numTags !=  cmp_tags._numTags)
      return false;

    for (uint8_t i = 0; i < _numTags; ++i) {
      if (!(_tags[i] == cmp_tags._tags[i]))
        return false;
    }
    return true;
  }
};


/* ************************************************************************* *
 * NIVERSAL CLASS TAG NUMBERS
 * ************************************************************************* */

extern ASTagging  _tagBOOL;             //1
extern ASTagging  _tagINTEGER;          //2
extern ASTagging  _tagBITSTR;           //3
extern ASTagging  _tagOCTSTR;           //4
extern ASTagging  _tagNULL;             //5
extern ASTagging  _tagObjectID;         //6
extern ASTagging  _tagObjDescriptor;    //7
extern ASTagging  _tagEXTERNAL;         //8
extern ASTagging  _tagREAL;             //9
extern ASTagging  _tagENUM;             //10
extern ASTagging  _tagEmbeddedPDV;      //11
extern ASTagging  _tagUTF8STR;          //12
extern ASTagging  _tagRelativeID;       //13

extern ASTagging  _tagSEQOF;            //16
extern ASTagging  _tagSETOF;            //17
extern ASTagging  _tagNumericSTR;       //18
extern ASTagging  _tagPrintableSTR;     //19
extern ASTagging  _tagTeletexSTR;       //20
extern ASTagging  _tagVideotexSTR;      //21
extern ASTagging  _tagIA5STR;           //22
extern ASTagging  _tagUTCTime;          //23
extern ASTagging  _tagGeneralizedTime;  //24
extern ASTagging  _tagGraphicSTR;       //25
extern ASTagging  _tagVisibleSTR;       //26
extern ASTagging  _tagGeneralSTR;       //27

extern ASTagging  _tagCHARSTR;          //29
extern ASTagging  _tagBMPSTR;           //30

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

