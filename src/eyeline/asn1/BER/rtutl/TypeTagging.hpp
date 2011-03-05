/* ************************************************************************* *
 * BER Decoder: Generic type tagging options.
 * ************************************************************************* */
#ifndef __ASN1_BER_TYPE_TAGGING
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TYPE_TAGGING

#include <map>

#include "eyeline/asn1/ASNTags.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTagging;


/* ************************************************************************* *
 * Helper class that handles type tagging options.
 * NOTE: only untagged CHOICE has more than one option.
 * ************************************************************************* */
//TODO: migrate to sorted LWArray_T (preallocated) and std::binary_search()
class TaggingOptions {
public:
  typedef std::map<ASTag, ASTagging>  TagsMAP;
  typedef TagsMAP::size_type          size_type;

protected:
  TagsMAP           _optTags;
  const ASTagging * _effTags; //selected effective tagging

public:
  TaggingOptions() : _effTags(0)
  { }
  explicit TaggingOptions(const ASTagging & use_tags, bool do_select = true);
  //
  TaggingOptions(const TaggingOptions & use_obj);
  //
  ~TaggingOptions()
  { }

  void clear(void) { _optTags.clear(); _effTags = 0; }

  //Adds given tagging to type tagging options
  const ASTagging * addTagging(const ASTagging & use_tags);

  //Adds given tagging to type tagging options
  const ASTagging * addTagging(const ASTag & use_tag, ASTagging::Environment_e tag_env)
  {
    return addTagging(ASTagging(use_tag, tag_env));
  }
  //Sets given tagging as a selected type tagging option
  const ASTagging * setTagging(const ASTagging & use_tags)
  {
    return (_effTags = addTagging(use_tags));
  }
  //Sets given tagging as a selected type tagging option
  const ASTagging * setTagging(const ASTag & use_tag, ASTagging::Environment_e tag_env)
  {
    return setTagging(ASTagging(use_tag, tag_env));
  }

  //
  void addOptions(const TaggingOptions & use_opt)
  {
    _optTags.insert(use_opt.getMap().begin(), use_opt.getMap().end());
  }
  //Select tagging option identified by given tag as an effective one.
  //Returns false if there is no option found
  //NOTE: in case of failure the effective tagging isn't defined! 
  bool selectTagging(const ASTag & use_tag);
  //
  const ASTagging * getEffective(void) const { return _effTags; }

  //Returns the outermost tag of effective option
  const ASTag * getTag(void) const
  {
    return _effTags ? &(_effTags->outerTag()) : 0;
  }

  //Returns the least tag of all options
  const ASTag * getCanonicalTag(void) const
  {
    return !_optTags.empty() ? &(_optTags.begin()->first) : 0;
  }
  //
  size_type size(void) const { return _optTags.size(); }
  //
  const TagsMAP & getMap(void) const { return _optTags; }
};

typedef TaggingOptions::TagsMAP TaggingOptionsMAP;

//NOTE: Complete tagging of CHOICE/ANY types may be calculated only after value
//      is to encode is known. In that case use method setOptions().
class TypeTagging {
private:
  ASTagging::size_type    _isTagged;
  const TaggingOptions *  _optTags; //set if base type is untagged CHOICE/Opentype,
                                    //i.e., has several tagging options
  ASTagging   _fullTags;  //complete tagging of type: {[type_tag +] base_tagging }

protected:
  //NOTE: in case of CHOICE/Opentype the copying constructor of successsor
  //      MUST properly set _optTags by setOptions().
  explicit TypeTagging(const TypeTagging & use_obj)
    : _isTagged(use_obj._isTagged), _optTags(0), _fullTags(use_obj._fullTags)
  { }
  //
  void setOptions(const TaggingOptions & use_opts)
  {
    _optTags = &use_opts;
    refreshTagging();
  }

public:
  //Generic untagged type tagging
  TypeTagging() : _isTagged(0), _optTags(0)
  { }

  //Ordinary type tagging (type is/or_references ordinary type)
  TypeTagging(ASTag use_tag, ASTagging::Environment_e use_env)
    : _isTagged(1), _optTags(0), _fullTags(use_tag, use_env)
  { }
  explicit TypeTagging(const ASTagging & base_tags)
    : _isTagged(base_tags.size()), _optTags(0), _fullTags(base_tags)
  { }
  //Tagged type tagging (type references ordinary type)
  TypeTagging(ASTag use_tag, ASTagging::Environment_e use_env,
              const ASTagging & base_tags)
    : _isTagged(1), _optTags(0), _fullTags(use_tag, use_env, base_tags)
  { }
  //
  ~TypeTagging()
  { }

  //recomposes complete tagging, meaningfull only if base type is untagged CHOICE
  const ASTagging * refreshTagging(void);

  bool isTagged(void) const { return _isTagged != 0; }

  const ASTagging * getTagging(void) const
  {
    return _fullTags.empty() ? 0 : &_fullTags;
  }
  //
  const ASTag * getTag(void) const
  {
    return _fullTags.empty() ? 0 : &_fullTags.outerTag();
  }
  //
  const ASTag * getCanonicalTag(void) const
  {
    return (!_isTagged && _optTags) ? _optTags->getCanonicalTag() : getTag();
  }
  //
  const TaggingOptions * getTaggingOptions(void) const { return _optTags; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TYPE_TAGGING */

