#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-Arg.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Arg ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ... ,
        alertingPattern         AlertingPattern OPTIONAL,
        msisdn              [0] ISDN-AddressString  OPTIONAL 
} */

static const asn1::ASTag  _tag_f3(asn1::ASTag::tagContextSpecific, 0);

//Initializes ElementDecoder for this type
void MDUSSD_Arg::construct(void)
{
  asn1::ber::DecoderOfSequence_T<5>::setField(0, *_dcs.getTag(),
                                              asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<5>::setField(1, *_ussd.getTag(),
                                              asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<5>::setField(2, *_alrtPtrn.getTag(),
                                              asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<5>::setField(3, _tag_f3, asn1::ASTagging::tagsIMPLICIT,
                                              asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<5>::setUnkExtension(4);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  MDUSSD_Arg::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("map::ss::dec::MDUSSD_Arg : value isn't set!");
  if (unique_idx > 4)
    throw smsc::util::Exception("map::ss::dec::MDUSSD_Arg::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _dcs.setValue(_dVal->ussd_DataCodingScheme);
    return &_dcs;
  }
  if (unique_idx == 1) {
    _ussd.setValue(_dVal->ussd_String);
    return &_ussd;
  }
  if (unique_idx == 2) {
    _alrtPtrn.setValue(_dVal->alertingPattern._value);
    return &_alrtPtrn;
  }
  if (unique_idx == 3) {
    _msIsdn.setValue(_dVal->msisdn);
    return &_msIsdn;
  }
  //if (unique_idx == 4)
  _uext.setValue(_dVal->_unkExt);
  return &_uext;
}

//Performs actions upon successfull optional element decoding
void MDUSSD_Arg::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  if (unique_idx == 2)
    _dVal->_fieldsMask |= USSD_Arg::has_alertingPattern;

  if (unique_idx == 3)
    _dVal->_fieldsMask |= USSD_Arg::has_msisdn;
}


}}}}

