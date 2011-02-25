#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/DecodeEmbdEnc.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class DecoderOfEmbdEncoding implementation:
 * ************************************************************************* */
const DecoderOfEmbdEncoding::TaggingOptions DecoderOfEmbdEncoding::_tagOptions;
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

  if (!unique_idx)
    _altDec.astype().init(_tagSingleASN1Type, ASTagging::tagsEXPLICIT, getTSRule()).setValue(_dVal->TS().init());
  else if (unique_idx == 1)
    _altDec.octstr().init(_tagOctetAligned, ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->OCTS().init());
  else //if (unique_idx == 2) {}
    _altDec.bitstr().init(_tagBitAligned, ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->BITS().init());
  return _altDec.get();
}

} //ber
} //asn1
} //eyeline

