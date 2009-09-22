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

  const ASTag * get(void) const { return _tags; }

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

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TAGS_DEFS__ */

