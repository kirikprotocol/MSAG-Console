/* ************************************************************************* *
 * Base classes that implements generic ASN.1 type
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_TYPE_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_TYPE_DEFS__

#include <set>
#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"
#include "util/Exception.hpp"

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
struct BITBuffer {
  uint8_t *       ptr;
  uint32_t        size;

  uint8_t         bitsGap;    //unused bits in first byte of encoding
  uint8_t         bitsUnused; //unused bits in last byte of encoding

  BITBuffer(uint8_t * use_ptr = 0, uint32_t use_sz = 0)
    : ptr(use_ptr), size(use_sz), bitsGap(0), bitsUnused(0)
  { }
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

class ASN1CodecError : public smsc::util::Exception {
public:
  ASN1CodecError(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

//Generic abstract type, implements ABSTRACT-SYNTAX.&Type
class ASTypeAC {
public:
  enum Presentation {
    valNone = 0, valEncoded, valMixed, valDecoded
  };
  typedef TransferSyntax::Rule EncodingRule;

protected:
  Presentation  valPresentation;
  BITBuffer     valEnc; //meaningfull only in case of valPresentation == valMixed
  EncodingRule  valRule;
  ASTagOptions  tags;

public:
  ASTypeAC()
    : valPresentation(valNone), valRule(undefinedER)
  { }
  ASTypeAC(ASTag::TagClass_e tag_class,  uint16_t tag_val)
    : valPresentation(valNone), valRule(undefinedER)
    , tags(ASTagging(ASTag(tag_class, tag_val)))
  { }
  ASTypeAC(const ASTagging & use_tags)
    : valPresentation(valNone), valRule(undefinedER)
    , tags(use_tags)
  { }
  virtual ~ASTypeAC()
  { }

  ASTagOptions & asTags(void) { return tags; }

  //type is tagged by single tag
  void setTagging(ASTag::TagClass_e tag_class,  uint16_t tag_val)
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
    valPresentation = valEncoded; valEnc = use_buf; valRule = use_rule;
  }

  //Returns type tag (outermost tag  of selected tagging option)
  const ASTag * Tag(void) const { return tags.Tag(); }
  //Returns type tagging (selected tagging option)
  const ASTagging * Tagging(void) const { return tags.Selected(); }
  //
  Presentation getPresentation(void) const { return valPresentation; }
  //
  const BITBuffer * getEncoding(void) const { return valEnc.ptr ? &valEnc : 0; }
  //
  EncodingRule getRule(void) const { return valRule; }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  virtual ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = ruleDER)
  /*throw ASN1CodecError*/ = 0;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context
  virtual DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = ruleDER)
  /*throw ASN1CodecError*/ = 0;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context
  virtual DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = ruleDER)
  /*throw ASN1CodecError*/ = 0;

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  virtual ENCResult encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER) {
    throw ASN1CodecError("encode(BITBuffer &): not implemented");
  }

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  virtual DECResult decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER) {
    throw ASN1CodecError("decode(BITBuffer &): not implemented");
  }
  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  virtual DECResult deferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER) {
    throw ASN1CodecError("defferedDecode(BITBuffer &): not implemented");
  }

  // ---------------------------------
  // ASTypeAC auxiliary methods
  // ---------------------------------
  //REQ: presentation == valEncoded | valMixed,
  //    (setEncoding() or DeferredDecode() was called)
  //OUT: type presentation (include all subcomponents) = valDecoded
  //NOTE: in case of decMoreInput, stores decoding context
  DECResult selfDecode(void) /*throw ASN1CodecError*/
  {
    if ((valPresentation == valEncoded) || (valPresentation == valMixed)) {
      BITBuffer useEnc = valEnc;
      return decode(useEnc, valRule);
    }
    return DECResult(decBadVal);
  }

  //REQ: presentation == valEncoded (setEncoding() was called)
  //OUT: presentation = valMixed | valDecoded, all 
  //     subcomponents presentation = valMixed | valDecoded
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult selfDeferredDecode(void) /*throw ASN1CodecError*/
  {
    if (valPresentation == valEncoded) {
      BITBuffer useEnc = valEnc;
      return deferredDecode(useEnc, valRule);
    }
    return DECResult(decBadVal);
  }
};

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_TYPE_DEFS__ */

