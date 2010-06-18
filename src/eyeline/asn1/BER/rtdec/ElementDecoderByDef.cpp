#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/ElementDecoderByDef.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class ElementDecoderByTDef implementation:
 * ************************************************************************* */
//NOTE: throws in case of multiple tag occurences detected
void ElementDecoderByTDef::addTag2Row(
  TDMatrixRow & td_row, const ASTag & use_tag, uint16_t alt_idx)
  /*throw(std::exception)*/
{
  uint16_t atPos = td_row.insert(TDEntry(use_tag, alt_idx));
  if (atPos < (td_row.size() - 1)) { //check tag uniqueness
    const TDEntry & tdEn = td_row.get()[atPos + 1];
    if (tdEn._tag == use_tag)
      throw ElementDecoderAC::DuplicateTagException(use_tag, tdEn._altIdx, alt_idx);
  }
}

//NOTE: throws in case of multiple tag occurences detected
void ElementDecoderByTDef::addTags2Row(
  TDMatrixRow & td_row, const EDAlternative & use_alt)
  /*throw(std::exception)*/
{
  if (use_alt.isChoice()) {
    const TaggingOptions::TagsMAP & tagsMap = *use_alt.getTagOpts();

    for (TaggingOptions::TagsMAP::const_iterator
          cit = tagsMap.begin(); cit != tagsMap.end(); ++cit)
      addTag2Row(td_row, cit->first, use_alt.getUId());
  } else
    addTag2Row(td_row, *use_alt.getTag(), use_alt.getUId());
}

//Returns index of next alternative is to process
//NOTE: throws in case of multiple tag occurences detected
uint16_t ElementDecoderByTDef::buildTDMatrixRow(
  uint16_t row_idx, uint16_t start_idx) /*throw(std::exception)*/
{
  uint16_t endIdx = start_idx, numTags = 0;
  //determine required size of row: count number of alternatives
  //up to either 1st mandatory alternative or end of array.
  while (endIdx < _altsArr->size()) {
    if (_altsArr->at(endIdx).empty())
      throw ElementDecoderAC::UndefinedUIdException(endIdx);
    if (!_altsArr->at(endIdx).isOptional())
      break;
    numTags += _altsArr->at(endIdx).numTags();
    ++endIdx;
  }

  TDMatrixRow & tdRow = _tdMatrix->at(row_idx);
  tdRow.reserve(numTags);

  for (; start_idx <= endIdx; ++start_idx) {
    const EDAlternative & alt = _altsArr->get()[start_idx];
    _occArr->at(start_idx)._tdRowIdx = row_idx;
    _occArr->at(start_idx)._isSingle = 
      (getBlocking(alt.getUId()) & EDAlternative::blockItself) != 0;
    addTags2Row(tdRow, alt);
  }

  //check for unatagged OpenType ambiguity
  if ((numTags > 1) && (tdRow.atLast()._tag == asn1::_tagANYTYPE))
    throw ElementDecoderAC::AmbiguousCtxException(tdRow.atLast()._altIdx);

  return endIdx + 1;
}


// ----------------------------------------------------------
// ElementDecoderAC interface methods (protected)
// ----------------------------------------------------------
//Returns blocking mode for alternative with given UId
uint8_t ElementDecoderByTDef::getBlocking(uint16_t alt_uid) const /*throw(std::exception)*/
{
  return _altsArr->at(alt_uid).isUnkExtension() ? EDAlternative::blockPreceeding :
              EDAlternative::blockItself | EDAlternative::blockPreceeding;
}

//Generates TagDecoder initialization data
void ElementDecoderByTDef::buildTagDecoder(void)  /* throw(std::exception)*/
{
  uint16_t idxAlt = 0, idxRow = 0;
  /**/
  while ((idxAlt = buildTDMatrixRow(idxRow, idxAlt)) < _altsArr->size())
    ++idxRow;
}
//Reverts TagDecoder runtime data to its 'just-built' state
void ElementDecoderByTDef::resetTagDecoder(void)
{
  _occMask->clear(); _fieldIdx = 0;
}

//Destroys all TagDecoder data
void ElementDecoderByTDef::eraseTagDecoder(void) /*throw()*/
{
  _occArr->clear(); 
  _tdMatrix->clear();
}

//Checks if occurence of alternative with given tag is legal at current
//ElementDecoderAC state updating it if necessary.
ElementDecoderAC::EDAResult
  ElementDecoderByTDef::processElementTag(const ASTag & use_tag)
  /*throw(std::exception)*/
{
  if (_fieldIdx >= _occArr->size())  //all elements already decoded
    return EDAResult(edaUnknown, &(_altsArr->at(_fieldIdx)));

  const TDMatrixRow & tdRow = _tdMatrix->at(_occArr->at(_fieldIdx)._tdRowIdx);
  uint16_t tdePos = tdRow.find(TDEntry(use_tag, 0));

  if (tdePos < tdRow.size()) { //tag found
    uint16_t altIdx = tdRow[tdePos]._altIdx;
    if (_occMask->at(altIdx) && _occArr->at(altIdx)._isSingle) //duplicate element occurence
      return EDAResult(edaMultiple, &(_altsArr->at(altIdx)));

    _occMask->setBit(altIdx);
    if (_occArr->at(altIdx)._isSingle) //advance to next element
      ++_fieldIdx;
    return EDAResult(edaOk, &(_altsArr->at(altIdx)));
  }
  //unknown tag -> check for untagged_Opentype/Unknown_Extensions_Entry
  //NOTE: Unknown_Extensions_Entry always sorted in first position,
  //      and untagged_Opentype may be the only TDEntry in row.
  const EDAlternative & alt = _altsArr->at(tdRow[0]._altIdx);
  return (alt.isOpentype() || alt.isUnkExtension()) ?
                  EDAResult(edaOk, &alt) : EDAResult(edaUnknown);
}

//Verifies that ElementDecoderAC is in complete state - all mandatory
//alternatives are processed according to its decoding order.
ElementDecoderAC::EDAResult
  ElementDecoderByTDef::verifyTDCompletion(void) const /*throw()*/
{ 
  //checks that no mandatory elements following last processed one are left undecoded
  for (uint16_t idx = _fieldIdx; idx < _altsArr->size(); ++idx) {
    if (!_altsArr->at(idx).isOptional() && !_occMask->at(idx))
      return EDAResult(edaMisssed, &_altsArr->at(idx));
  }
  return EDAResult(edaOk);
}

} //ber
} //asn1
} //eyeline

