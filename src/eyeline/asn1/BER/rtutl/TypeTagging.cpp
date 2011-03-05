#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtutl/TypeTagging.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class TaggingOptions implementation:
 * ************************************************************************* */
TaggingOptions::TaggingOptions(const ASTagging & use_tags, bool do_select/* = true*/)
  : _effTags(0)
{
  if (do_select)
    setTagging(use_tags);
  else
    addTagging(use_tags);
}

TaggingOptions::TaggingOptions(const TaggingOptions & use_obj)
  : _optTags(use_obj._optTags), _effTags(0)
{
  if (use_obj._effTags)
    selectTagging(use_obj._effTags->outerTag());
}

const ASTagging * TaggingOptions::addTagging(const ASTagging & use_tags)
{
  std::pair<TagsMAP::iterator, bool> res = 
    _optTags.insert(TagsMAP::value_type(use_tags.outerTag(), use_tags));
  return &(res.first->second);
}

bool TaggingOptions::selectTagging(const ASTag & use_tag)
{
  TagsMAP::const_iterator cit = _optTags.find(use_tag);
  if (cit != _optTags.end()) {
    _effTags = &(cit->second);
    return true;
  }
  _effTags = 0;
  return false;
}

/* ************************************************************************* *
 * Class TypeTagging implementation:
 * ************************************************************************* */
const ASTagging * TypeTagging::refreshTagging(void)
{
  if (_optTags) {
    //cut off tags previously inherited from referenced type
    _fullTags.resize(_isTagged);
    if (_optTags->getEffective()) {
      if (_isTagged)
        _fullTags.append(*(_optTags->getEffective()));
      else
        _fullTags = *(_optTags->getEffective());
    }
  }
  return getTagging();
}

} //ber
} //asn1
} //eyeline

