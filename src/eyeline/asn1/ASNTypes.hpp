/* ************************************************************************* *
 * Base classes that implements generic ASN.1 type
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TYPE_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TYPE_DEFS__

#include <stdarg.h>
#include <set>
#include <vector>

namespace eyeline {
namespace asn1 {

struct OCTBuffer {
  uint8_t *       ptr;
  uint32_t        size;

  OCTBuffer(uint8_t * use_ptr = 0, uint32_t use_sz = 0)
    : ptr(use_ptr), size(use_sz)
  { }
};

//unaligned BIT buffer
struct BITBuffer : public OCTBuffer {
  uint8_t         bitsGap;    //unused bits in first byte of encoding
  uint8_t         bitsUnused; //unused bits in last byte of encoding

  BITBuffer(uint8_t * use_ptr = 0, uint32_t use_sz = 0)
    : OCTBuffer(use_ptr, use_sz), bitsUnused(0), bitsGap(0)
  { }
};

struct ASTag {
  enum TagClass {
    tagUniversal = 0x00,
    tagApplication = 0x40,
    tagContextSpecific = 0x80,
    tagPrivate = 0xB0
  };

  TagClass    tagClass;
  uint16_t    tagValue;

  ASTag(TagClass tag_class = tagUniversal, uint16_t tag_val = 0)
    : tagClass(tag_class), tagValue(tag_val)
  { }

  bool operator== (const ASTag & cmp_tag) const
  {
    return (tagClass == cmp_tag.tagClass) && (tagValue == cmp_tag.tagValue);
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
    if (tagClass == cmp_tag.tagClass)
        return (tagValue < cmp_tag.tagValue);
    return (tagClass < cmp_tag.tagClass);
  }
};

//Abstract type complete tagging (all tags goes to encoding in case of
//EXPLICIT tagging environment).
class ASTagging : std::vector<ASTag> {
public:
  ASTagging()    //just a [UNIVERSAL 0]
    : std::vector<ASTag>(1)
  { }
  ASTagging(const ASTag & use_tag) //just one tag
    : std::vector<ASTag>(1)
  {
    at(0) = use_tag;
  }
  ASTagging(const ASTagging & use_tags)
    : std::vector<ASTag>(use_tags)
  { }
  ASTagging(uint16_t num_tags, ASTag use_tag1, ... /* , const ASTag use_tagN*/)
    : std::vector<ASTag>(num_tags)
  {
    at(0) = use_tag1;
    va_list  useTags;
    va_start(useTags, use_tag1);
    for (uint16_t i = 1; i < num_tags; ++i)
      at(i) = va_arg(useTags, ASTag);
    va_end(useTags);
  }
  ~ASTagging()
  { }


  uint16_t numTags(void) const
  {
    return (uint16_t)(size());
  }
  //tag_idx = 0 - outermost tag, in most cases this is just a type tag
  //NOTE: It's a caller responsibility to ensure tag_idx isn't out of range
  const ASTag & Tag(uint16_t tag_idx = 0) const
  {
    return tag_idx < numTags() ? at(tag_idx) : at(0);
  }

  //Compares taggings in accordance with ASN.1 canonical tags order.
  bool operator< (const ASTagging & cmp_tags) const
  {
    return at(0) < cmp_tags.Tag(0);
  }
};

//This class helps to cope with untagged CHOICE types, which potentially may have
//several tagging options, but only one at a time depending on CHOICE value type
//assigned.
//NOTE: tagging options are sorted in accordance with ASN.1 canonical tags order.
class ASTagOptions : std::set<ASTagging> {
protected:
  std::set<ASTagging>::const_iterator current;

public:
  typedef std::set<ASTagging>::const_iterator  const_iterator;

  ASTagOptions()
  {
    current = end();
  }
  ASTagOptions(const ASTagOptions & use_opt)
    : std::set<ASTagging>(use_opt)
  {
    current = use_opt.Selected() ? find(use_opt.Selected()->Tag(0)) : end();
  }
  //NOTE: sets selected option!
  ASTagOptions(const ASTagging & use_tags)
  {
    addOption(use_tags, true);
  }
  ~ASTagOptions()
  { }

  uint16_t numOptions(void) const { return (uint16_t)size(); }

  const_iterator first(void) const { return begin(); }
  const_iterator last(void) const  { return end(); }

  //Searches for tagging option with iutermost tag equal to given one
  const ASTagging * getOption(const ASTag & out_tag) const
  {
    const_iterator it = find(ASTagging(out_tag));
    return (it == end()) ? 0 : &*it;
  }
  //Returns selected tagging option, if latter is set
  const ASTagging * Selected(void) const
  {
    return (current == end()) ? 0 : &(*current);
  }
  //Returns outermost tag of selected tagging option, if latter is set
  const ASTag * Tag(void) const
  {
    return Selected() ? &(Selected()->Tag(0)) : 0;
  }

  //Adds tagging option, optionally setting it as selected one.
  bool addOption(const ASTagging & use_tags, bool do_select = false)
  {
    std::pair<std::set<ASTagging>::iterator, bool> res = insert(use_tags);
    if (res.second && do_select)
      current = res.first;
    return res.second;
  }
  //Sets option with outermost tag equal to given one as selected.
  bool selectOption(const ASTag & out_tag)
  {
    current = find(ASTagging(out_tag));
    return  current != end();
  }
};

//Generic abstract type, implements ABSTRACT-SYNTAX.&Type
class ASTypeAC {
public:
  enum Presentation {
    valNone = 0, valEncoded, valMixed, valDecoded
  };
  enum EncodingRule {
    undefinedER = 0
    //octet aligned encodings:
    , ruleBER   //ruleBasic
    , ruleDER   //ruleDistinguished
    , ruleCER   //ruleCanonical
    , rulePER   //rulePacked_aligned
    , ruleCPER  //rulePacked_aligned_canonical
    , ruleXER   //ruleXml
    , ruleCXER  //ruleXml_canonical
    , ruleEXER  //ruleXml_extended
    //bit aligned encodings:
    , ruleUPER  //rulePacked_unaligned
    , ruleCUPER //rulePacked_unaligned_canonical
  };

  enum ENCStatus {
    encBadVal = -2, encError = -1, encOk = 0, encMoreMem = 1
  };
  enum DECStatus {
    decBadVal = -2, decError = -1, decOk = 0, decMoreInput = 1
  };

  struct ENCResult {
    ENCStatus   rval;   //encoding status
    uint32_t    nbytes; //number of bytes encoded

    ENCResult() : rval(encOk), nbytes(0)
    { }
  };
  struct DECResult {
    DECStatus   rval;   //decoding status
    uint32_t    nbytes; //number of bytes succsefully decoded

    DECResult() : rval(decOk), nbytes(0)
    { }
  };

protected:
  Presentation  valType;
  BITBuffer     valEnc;
  EncodingRule  valRule;
  ASTagOptions  tags;

public:
  ASTypeAC()
    : valType(valNone), valRule(undefinedER)
  { }
  ASTypeAC(ASTag::TagClass tag_class,  uint16_t tag_val)
    : valType(valNone), valRule(undefinedER)
    , tags(ASTagging(ASTag(tag_class, tag_val)))
  { }
  ASTypeAC(const ASTagging & use_tags)
    : valType(valNone), valRule(undefinedER)
    , tags(use_tags)
  { }
  virtual ~ASTypeAC()
  { }

  ASTagOptions & asTags(void) { return tags; }

  //type is tagged by single tag
  void setTagging(ASTag::TagClass tag_class,  uint16_t tag_val)
  {
    tags.selectOption(ASTag(tag_class, tag_val));
  }
  //NOTE: use_tags cann't be empty !
  void setTagging(const ASTagOptions & use_tags)
  {
    tags.selectOption(*use_tags.Tag());
  }

  void setEncoding(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
  {
    valType = valEncoded; valEnc = use_buf; valRule = use_rule;
  }

  //Returns type tag (outermost tag  of selected tagging option)
  const ASTag * Tag(void) const { return tags.Tag(); }
  //Returns type tagging (selected tagging option)
  const ASTagging * Tagging(void) const { return tags.Selected(); }
  //
  Presentation getPresentation(void) const { return valType; }
  //
  const BITBuffer * getEncoding(void) const { return valEnc.ptr ? &valEnc : 0; }
  //
  EncodingRule getRule(void) const { return valRule; }

  // ---------------------------------
  // ASTypeAC interface methods
  // ---------------------------------

  //REQ: presentation > valNone, if use_rule == valRule, otherwise presentation == valDecoded
  virtual ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER) /*throw ASN1CodecError*/ = 0;
  //REQ: presentation == valEncoded | valMixed (setEncoding was called)
  //OUT: type presentation = valDecoded, components (if exist) presentation = valDecoded,
  //in case of decMoreInput, stores decoding context
  virtual DECResult Decode(void) /*throw ASN1CodecError*/ = 0;
  //REQ: presentation == valEncoded  (setEncoding was called)
  //OUT: type presentation = valMixed | valDecoded, 
  //     deferred components presentation = valEncoded
  //NOTE: all components decoding is deferred 
  //in case of decMoreInput, stores decoding context 
  virtual DECResult DeferredDecode(void) /*throw ASN1CodecError*/ = 0;


  // ---------------------------------
  // ASTypeAC auxiliary methods
  // ---------------------------------
  //REQ: presentation == valNone, if decMoreInput stores decoding context,
  //type presentation = valDecoded, components (if exist) presentation = valDecoded
  DECResult DecodeBuf(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
      /*throw ASN1CodecError*/
  {
    setEncoding(use_buf, use_rule);
    return Decode();
  }
  //REQ: presentation == valNone, if decMoreInput stores decoding context
  //type presentation = valDecoded, deferred components presentation = valEncoded
  //NOTE: if num_tags == 0 all components are deferred 
  DECResult DeferredDecodeBuf(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
      /*throw ASN1CodecError*/
  {
    setEncoding(use_buf, use_rule);
    return DeferredDecode();
  }
};


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TYPE_DEFS__ */

