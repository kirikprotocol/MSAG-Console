/* ************************************************************************* *
 * BER Decoder: Generic type tagging options.
 * ************************************************************************* */
#ifndef __ASN1_BER_TYPE_TAGGING
#ident "@(#)$Id$"
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
  TaggingOptions(const ASTagging & use_tags, bool do_select = true)
   : _effTags(0)
  {
    std::pair<TagsMAP::iterator, bool> res = 
      _optTags.insert(TagsMAP::value_type(use_tags.outerTag(), use_tags));
    if (do_select)
      _effTags = &(res.first->second);
  }
  TaggingOptions(const TaggingOptions & use_obj)
    : _optTags(use_obj._optTags), _effTags(0)
  {
    if (use_obj._effTags)
      selectTagging(use_obj._effTags->outerTag());
  }

  ~TaggingOptions()
  { }

  void clear(void) { _optTags.clear(); _effTags = 0; }

  //Adds given tagging to type tagging options
  const ASTagging * addTagging(const ASTag & use_tag, ASTagging::Environment_e tag_env)
  {
    std::pair<TagsMAP::iterator, bool> res = 
      _optTags.insert(TagsMAP::value_type(use_tag, ASTagging(use_tag, tag_env)));
    return &(res.first->second);
  }
  //Adds given tagging to type tagging options
  const ASTagging * addTagging(const ASTagging & use_tags)
  {
    std::pair<TagsMAP::iterator, bool> res = 
      _optTags.insert(TagsMAP::value_type(use_tags.outerTag(), use_tags));
    return &(res.first->second);
  }

  //Sets given tagging as a selected type tagging option
  const ASTagging * setTagging(const ASTag & use_tag, ASTagging::Environment_e tag_env)
  {
    return (_effTags = addTagging(use_tag, tag_env));
  }
  //Sets given tagging as a selected type tagging option
  const ASTagging * setTagging(const ASTagging & use_tags)
  {
    return (_effTags = addTagging(use_tags));
  }
  //
  void addOptions(const TaggingOptions & use_opt)
  {
    _optTags.insert(use_opt.getMap().begin(), use_opt.getMap().end());
  }
  //Select tagging option identified by given tag as an effective one.
  //Returns false if there is no option found
  //NOTE: in case of failure the effective tagging isn't defined! 
  bool selectTagging(const ASTag & use_tag) //throw(std::exception)
  {
    TagsMAP::const_iterator cit = _optTags.find(use_tag);
    if (cit != _optTags.end()) {
      _effTags = &(cit->second);
      return true;
    }
    _effTags = 0;
    return false;
  }
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


class TypeTagging {
private:
  ASTagging::size_type    _isTagged;
  const TaggingOptions *  _optTags; //set if base type is untagged CHOICE/Opentype,
                                    //i.e., has several tagging options
  ASTagging   _fullTags;  //complete tagging of type: {[type_tag +] base_tagging }

protected:
  //NOTE: in case of CHOICE/Opentype the copying constructor of successsor
  //      MUST properly set _optTags by setOptions().
  TypeTagging(const TypeTagging & use_obj)
    : _isTagged(use_obj._isTagged), _optTags(0), _fullTags(use_obj._fullTags)
  { }
  //
  void setOptions(const TaggingOptions & use_opts) { _optTags = &use_opts; }

public:
  //Ordinary type tagging (type is/or_references ordinary type)
  TypeTagging(const ASTagging & base_tags)
    : _isTagged(base_tags.size()), _optTags(0), _fullTags(base_tags)
  { }
  //Untagged CHOICE/Opentype type tagging
  TypeTagging(const TaggingOptions * base_tags = 0)
    : _isTagged(0), _optTags(base_tags)
  {
    if (_optTags && _optTags->getEffective())
      _fullTags = *(_optTags->getEffective());
  }
  //Tagged type tagging (type references ordinary type)
  TypeTagging(ASTag use_tag, ASTagging::Environment_e use_env,
              const ASTagging & base_tags)
    : _isTagged(1), _optTags(0), _fullTags(use_tag, use_env, base_tags)
  { }
  //Tagged type tagging (type references untagged CHOICE/Opentype)
  TypeTagging(ASTag use_tag, ASTagging::Environment_e use_env,
              const TaggingOptions & base_tags)
    : _isTagged(1), _optTags(&base_tags), _fullTags(use_tag, use_env)
  {
    if (_optTags->getEffective())
      _fullTags.conjoin(*(_optTags->getEffective()));
  }
  //Tagged type tagging (type references untagged CHOICE/Opentype)
  TypeTagging(const ASTagging & use_tags,
              const TaggingOptions & base_tags)
    : _isTagged(use_tags.size()), _optTags(&base_tags), _fullTags(use_tags)
  {
    if (_optTags->getEffective())
      _fullTags.conjoin(*(_optTags->getEffective()));
  }

  ~TypeTagging()
  { }

  //recomposes complete tagging, meaningfull only if base type is untagged CHOICE
  const ASTagging * refreshTagging(void)
  {
    if (_optTags) {
      //cut off tags previously inherited from referenced type
      _fullTags.resize(_isTagged);
      if (_optTags->getEffective()) {
        if (_isTagged)
          _fullTags.conjoin(*(_optTags->getEffective()));
        else
          _fullTags = *(_optTags->getEffective());
      }
    }
    return getTagging();
  }

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

