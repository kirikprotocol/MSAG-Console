#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-Res.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Res ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ...
} */
//Initializes ElementDecoder for this type
void MDUSSD_Res::construct(void)
{
  asn1::ber::DecoderOfSequence_T<3>::setField(0, *_dcs.getTag(),
                                              asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setField(1, *_ussd.getTag(),
                                              asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setUnkExtension(2);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  MDUSSD_Res::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("map::ss::dec::MDUSSD_Res : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("map::ss::dec::MDUSSD_Res::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _dcs.setValue(_dVal->ussd_DataCodingScheme);
    return &_dcs;
  }
  if (unique_idx == 1) {
    _ussd.setValue(_dVal->ussd_String);
    return &_ussd;
  }
  //if (unique_idx == 2)
  _uext.init(getTSRule()).setValue(_dVal->_unkExt);
  return _uext.get();
}


}}}}

