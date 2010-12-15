/* ************************************************************************* *
 * BER Decoder: Structured type ElementDecoder alternative class.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ELEMENTS_DECODER_ALT_DEF
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_ELEMENTS_DECODER_ALT_DEF

#include "eyeline/asn1/BER/rtutl/TypeTagging.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//ElementDecoder alternative
class EDAlternative {
public:
  enum Kind_e {
      altNone = 0
    , altUExtension //uknon extension additions entry, has _tagUNI0 assigned
    , altOpentype   //untagged ANY/OpenType, has _tagANYTYPE
    , altChoice     //untagged CHOICE (several options possible)
    , altElement    //ordinary element occurence (has tag defined)
  };
  enum Blocking_e {
      blockNone = 0           //recurring alternative, doesn't affect others 
    , blockItself = 0x01      //single occurence alternative, doesn't affect others 
    , blockPreceeding = 0x02  //alternative blocks all preceeding alternatives
    , blockFollowing = 0x04   //alternative blocks all following alternatives
    , blockALL = 0x07         //single occurence alternative that blocks all others
  };
  enum Tagging_e {
      atgUntagged = 0 //alternative is identified by tag that is outermost tag
                      //of its complete tagging
    , atgImplicit     //alternative is identified by tag that replaces outermost
                      //tag of its complete tagging
    , atgExplicit     //alternative is identified by tag that isn't part of its
                      //complete tagging
  };
  enum Presence_e { altMANDATORY = 0, altOPTIONAL = 1 };

private:
  union {
    void * _aligner;
    uint8_t _buf[sizeof(ASTag)];
  } _memTag;

protected:
  Kind_e          _kind;
  uint16_t        _uniqueId;  //
  bool            _optional;  //presence mode
  Tagging_e       _tagEnv;
  union {
    const void *  _none;
    const ASTag * _single;    //in case of altOpentype == _tagUNI0, 
    const TaggingOptions::TagsMAP * _opts; //tagging options of associated type
  } _tag;                     

  void setTag(const ASTag & use_tag)
  {
    _tag._single = new (_memTag._buf)ASTag(use_tag);
  }
  void clearTag(void)
  {
    if ((_kind != altChoice) && _tag._single) {
      _tag._single->~ASTag();
      _tag._single = NULL;
    }
  }
  void copyTag(const EDAlternative & cp_obj)
  {
    _tag._none = _memTag._aligner = 0;
    if (_kind == altChoice)
      _tag._opts = cp_obj._tag._opts;
    else if (cp_obj._tag._single)
      setTag(*cp_obj._tag._single);
  }
public:
  //default constructor for containers
  EDAlternative()
    : _kind(altNone), _uniqueId(0), _optional(true), _tagEnv(atgUntagged)
  {
    _tag._none = _memTag._aligner = 0;
  }
  //copying constructor for containers
  EDAlternative(const EDAlternative & cp_obj)
    : _kind(cp_obj._kind), _uniqueId(cp_obj._uniqueId)
    , _optional(cp_obj._optional), _tagEnv(cp_obj._tagEnv)
  {
    copyTag(cp_obj);
  }

  //alternative of Unknown_extensions_entry
  EDAlternative(uint16_t unique_id)
    : _kind(altUExtension), _uniqueId(unique_id)
    , _optional(true), _tagEnv(atgUntagged)
  {
    setTag(asn1::_tagUNI0);
  }
  //untagged alternative of Opentype/ANY
  EDAlternative(uint16_t unique_id, Presence_e use_presence)
    : _kind(altOpentype), _uniqueId(unique_id)
    , _optional(use_presence != altMANDATORY), _tagEnv(atgUntagged)
  {
    setTag(asn1::_tagANYTYPE);
  }
  //untagged alternative of CHOICE option 
  EDAlternative(uint16_t unique_id, const TaggingOptions::TagsMAP & use_tag_opts,
                Presence_e use_presence = altMANDATORY)
    : _kind(altChoice), _uniqueId(unique_id)
    , _optional(use_presence != altMANDATORY), _tagEnv(atgUntagged)
  {
    _tag._opts = &use_tag_opts;
  }
  //untagged alternative of ordinary type, that has tag defined
  EDAlternative(uint16_t unique_id, const ASTag & use_tag,
                Presence_e use_presence = altMANDATORY)
    : _kind(altElement), _uniqueId(unique_id)
    , _optional(use_presence != altMANDATORY), _tagEnv(atgUntagged)
  {
    setTag(use_tag);
  }
  //tagged alternative of arbitrary type
  EDAlternative(uint16_t unique_id, const ASTag & use_tag, ASTagging::Environment_e tag_env,
                Presence_e use_presence = altMANDATORY)
    : _kind(altElement), _uniqueId(unique_id)
    , _optional(use_presence != altMANDATORY)
    , _tagEnv((tag_env == ASTagging::tagsEXPLICIT) ? atgExplicit : atgImplicit)
  {
    setTag(use_tag);
  }
  ~EDAlternative()
  {
    clearTag();
  }

  Kind_e getKind(void) const { return _kind; }
  //
  bool empty(void) const { return _kind == altNone; }
  //
  bool isChoice(void) const { return _kind == altChoice; }
  //
  bool isOpentype(void) const { return _kind == altOpentype; }
  //
  bool isUnkExtension(void) const { return _kind == altUExtension; }
  //
  uint16_t  getUId(void) const { return _uniqueId; }
  //
  Tagging_e getEnv(void) const { return _tagEnv; }
  //
  const ASTag * getTag(void) const { return isChoice() ? NULL : _tag._single; }
  //
  const TaggingOptions::TagsMAP * getTagOpts(void) const
  {
    return isChoice() ? _tag._opts : NULL;
  }
  //
  bool isOptional(void) const { return _optional; }

  //
  uint16_t numTags(void) const
  {
    return (_kind == altChoice) ? (uint16_t)(_tag._opts->size())
                                : (_tag._single ? 1 : 0);
  }

  void resetTag(const ASTag & use_tag)
  {
    clearTag();
    setTag(use_tag);
  }

  //
  bool operator<(const EDAlternative & cmp_obj) const
  {
    return _uniqueId < cmp_obj._uniqueId;
  }

  EDAlternative & operator=(const EDAlternative & cp_obj)
  {
    _kind = cp_obj._kind;
    _uniqueId = cp_obj._uniqueId;
    _optional = cp_obj._optional;
    _tagEnv = cp_obj._tagEnv;
    copyTag(cp_obj);
    return *this;
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ELEMENTS_DECODER_ALT_DEF */


