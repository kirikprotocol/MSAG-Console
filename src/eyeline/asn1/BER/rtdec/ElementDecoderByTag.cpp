#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/ElementDecoderByTag.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
/* ************************************************************************* *
 * Class ElementDecoderByTagAC implementation:
 * ************************************************************************* */

ElementDecoderByTagAC::TagOccurence *
  ElementDecoderByTagAC::insertTag(const ASTag & use_tag, const TagOccurence & alt_occ)
  /* throw(std::exception)*/
{
  { //check tag uniqueness
    EDATagsOrder::iterator it = _orderTags.find(use_tag);
    if (it != _orderTags.end())
      throw ElementDecoderAC::DuplicateTagException(use_tag, it->second.getAlt()->getUId(),
                                                    alt_occ.getAlt()->getUId());
  }
  std::pair<EDATagsOrder::iterator, bool> res = 
    _orderTags.insert(EDATagsOrder::value_type(use_tag, alt_occ));
  return &(res.first->second);
}

//Updates distingueshed order of alternative's tags
void ElementDecoderByTagAC::updateTagsRegistry(const EDAlternative & use_alt)
{
  TagOccurence  altOcc(use_alt);

  if (use_alt.isChoice()) {
    TaggingOptions::TagsMAP::const_iterator cit = use_alt.getTagOpts()->begin();

    //NOTE: only CHOICE has more than one tag, and the least one is canonical
    TagOccurence * occCan = insertTag(cit->first, altOcc);
    altOcc.setCanonical(occCan);
    while (++cit != use_alt.getTagOpts()->end())
      insertTag(cit->first, altOcc);
    /**/
  } else
    insertTag(*use_alt.getTag(), altOcc);
}

ElementDecoderAC::EDAStatus_e
  ElementDecoderByTagAC::blockAlternativeTags(EDATagsOrder::iterator & it_tag, bool only_canonical)
{
  TagOccurence &  occTag = it_tag->second;

  if (occTag.getAlt()->isUnkExtension())
    return ElementDecoderAC::edaOk;

  occTag._blocked = true;
  if (!occTag.isCanonical()) { //non-canonical alternative of untagged CHOICE
    //update optionsCounter of canonical alternative
    occTag.getCanonical()->_optCount += 1;
  }
  //check overall ocuurence number 
  if (occTag.numOccured(true) > 1)
    return ElementDecoderAC::edaMultiple;
    
  if (occTag.getAlt()->numTags() < 2)
    return ElementDecoderAC::edaOk;

  //Here goes only untagged CHOICE with several options.
  const TaggingOptions::TagsMAP * altTags = occTag.getAlt()->getTagOpts();

  //In case of orderCanonical block OTHER NON-CANONICAL alternatives of CHOICE
  //and mark them as optional, otherwise block ALL OTHER alternatives of CHOICE
  //and mark them as optional.
  for (TaggingOptions::TagsMAP::const_iterator cit = altTags->begin(); 
                                              cit != altTags->end(); ++cit) {
    if (cit->first != it_tag->first) {
      TagOccurence & occCit = _orderTags[cit->first];
      if (!(occCit.isCanonical() && only_canonical))
        occCit._blocked = occCit._optional = true;
    }
  }
  return ElementDecoderAC::edaOk;
}

//NOTE: it_tag != end() upon entry
ElementDecoderAC::EDAStatus_e
  ElementDecoderByTagAC::blockPreceedingByTag(EDATagsOrder::iterator it_tag)
{
  while (it_tag != _orderTags.begin()) {
    TagOccurence & occTag = (--it_tag)->second;
    if (occTag.isAvailable(_order == orderCanonical)) {
      if (!occTag._optional && !occTag.numOccured(_order == orderCanonical))
        return ElementDecoderByTagAC::edaMisplaced; //mandatory element missed
      occTag._blocked = true;
    }
  }
  return ElementDecoderByTagAC::edaOk;
}

//NOTE: it_tag != end() upon entry
ElementDecoderAC::EDAStatus_e
  ElementDecoderByTagAC::blockFollowingByTag(EDATagsOrder::iterator it_tag)
{
  while (++it_tag != _orderTags.end()) {
    TagOccurence & occTag = it_tag->second;
    occTag._blocked = true;
  }
  return ElementDecoderByTagAC::edaOk;
}

ElementDecoderAC::EDAStatus_e
  ElementDecoderByTagAC::processAlternative(EDATagsOrder::iterator & it_tag)
  /*throw(std::exception)*/
{
  TagOccurence & occTag = it_tag->second;

  //update alternative tag occurence information
  ++occTag._count;
  if (occTag._blocked)
    return (occTag._count > 1) ? ElementDecoderByTagAC::edaMultiple : ElementDecoderByTagAC::edaMisplaced;

  EDAStatus_e rval = ElementDecoderByTagAC::edaOk;

  uint8_t blockMode = getBlocking(occTag.getAltUId());

  if (blockMode & EDAlternative::blockItself) {
    if ((rval = blockAlternativeTags(it_tag, _order == orderCanonical)) != ElementDecoderByTagAC::edaOk)
      return rval;
  }
  if (blockMode & EDAlternative::blockPreceeding) {
    if ((rval = blockPreceedingByTag(it_tag)) != ElementDecoderByTagAC::edaOk)
      return rval;
  }
  if (blockMode & EDAlternative::blockFollowing)
    rval = blockFollowingByTag(it_tag);
  return rval;
}


// ----------------------------------------------------------
// ElementDecoderAC interface methods (protected)
// ----------------------------------------------------------
//Generates TagDecoder initialization data
void ElementDecoderByTagAC::buildTagDecoder(void)  /* throw(std::exception)*/
{
  for (uint16_t i = 0; i < _altsArr->size(); ++i) {
    if (_altsArr->get()[i].empty())
      throw ElementDecoderAC::UndefinedUIdException(i);
    updateTagsRegistry(*(_altsArr->get() + i));
  }
}

//Reverts TagDecoder runtime data to its 'just-built' state
void ElementDecoderByTagAC::resetTagDecoder(void)
{
  for (EDATagsOrder::iterator it = _orderTags.begin(); it != _orderTags.end(); ++it)
    it->second.reset();
}

//Destroys all TagDecoder data
void ElementDecoderByTagAC::eraseTagDecoder(void) /*throw()*/
{
  _orderTags.clear();
}

//Checks if occurence of alternative with given tag is legal at current
//ElementDecoderAC state updating it if necessary.
ElementDecoderAC::EDAResult
  ElementDecoderByTagAC::processElementTag(const ASTag & use_tag)
  /*throw(std::exception)*/
{
  EDAResult rval(ElementDecoderAC::edaOk);

  EDATagsOrder::iterator itTag = _orderTags.find(use_tag);
  if (itTag == _orderTags.end()) {
    if (!_extensible) {
      rval._status = ElementDecoderAC::edaUnknown;
      return rval;
    }
    //itTag = _orderTags.find(asn1::_tagUNI0);
    itTag = _orderTags.begin(); //_tagUNI0 is alway the least tag!
  }
  rval._alt = itTag->second.getAlt();
  rval._status = processAlternative(itTag);
  return rval;
}

//Verifies that ElementDecoderAC is in complete state - all mandatory
//alternatives are processed according to its decoding order.
ElementDecoderAC::EDAResult
  ElementDecoderByTagAC::verifyTDCompletion(void) const /*throw()*/
{
  EDATagsOrder::const_iterator it = _orderTags.end();

  while (it != _orderTags.begin()) {
    const TagOccurence & occTag = (--it)->second;
    if (occTag.isAvailable(_order == orderCanonical)) {
      if (!occTag._optional && !occTag.numOccured(_order == orderCanonical))
        return EDAResult(edaMisplaced, occTag.getAlt()); //mandatory element missed
    }
  }
  return EDAResult(edaOk);
}

} //ber
} //asn1
} //eyeline

