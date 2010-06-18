#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/DecodeEmbdEnc.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfEmbdEncoding implementation:
 * ************************************************************************* */
const ASTag DecoderOfEmbdEncoding::_tagSingleASN1Type(ASTag::tagContextSpecific, 0);
const ASTag DecoderOfEmbdEncoding::_tagOctetAligned(ASTag::tagContextSpecific, 1);
const ASTag DecoderOfEmbdEncoding::_tagBitAligned(ASTag::tagContextSpecific, 2);

//Initializes ElementDecoder of this type;
void DecoderOfEmbdEncoding::construct(void)
{
  DecoderOfChoice_T<3>::setAlternative(0, _tagSingleASN1Type, ASTagging::tagsEXPLICIT);
  DecoderOfChoice_T<3>::setAlternative(1, _tagOctetAligned, ASTagging::tagsIMPLICIT);
  DecoderOfChoice_T<3>::setAlternative(2, _tagBitAligned, ASTagging::tagsIMPLICIT);
}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
TypeDecoderAC * 
  DecoderOfEmbdEncoding::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ber::DecoderOfEmbdEncoding : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("ber::DecoderOfEmbdEncoding::prepareAlternative() : undefined UId");

  cleanUp();
  if (!unique_idx) {
    _altDec._astype = new (_memAlt._buf) DecoderOfASType(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule());
    _altDec._astype->setValue(_dVal->initTS());
    return _altDec._astype;
  }
  if (unique_idx == 1) {
    _altDec._octstr = new (_memAlt._buf) DecoderOfOCTSTR(_tagOctetAligned, ASTagging::tagsIMPLICIT, getTSRule());
    _altDec._octstr->setValue(_dVal->initOCTS());
    return _altDec._octstr;
  }
  //if (unique_idx == 2) {}
  _altDec._bitstr = new (_memAlt._buf) DecoderOfBITSTR(_tagBitAligned, ASTagging::tagsIMPLICIT, getTSRule());
  _altDec._bitstr->setValue(_dVal->initBITS());
  return _altDec._bitstr;
}

} //ber
} //asn1
} //eyeline

