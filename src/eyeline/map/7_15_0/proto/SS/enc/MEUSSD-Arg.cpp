#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-Arg.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

const asn1::ASTag  MEUSSD_Arg::_tag_f3(asn1::ASTag::tagContextSpecific, 0);

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Arg ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ... ,
        alertingPattern         AlertingPattern OPTIONAL,
        msisdn              [0] ISDN-AddressString  OPTIONAL 
} */

//inits mandatory fields encoders
void MEUSSD_Arg::construct(void)
{
  asn1::ber::EncoderOfSequence_T<4>::setField(0, _dcs);
  asn1::ber::EncoderOfSequence_T<4>::setField(1, _ussd);
}

//
void MEUSSD_Arg::setValue(const USSD_Arg & use_val) /*throw(std::exception)*/
{
  _dcs.setValue(use_val.ussd_DataCodingScheme);
  _ussd.setValue(use_val.ussd_String);

  if (use_val._fieldsMask & use_val.has_alertingPattern) {
    _alrtPtrn.setValue(use_val.alertingPattern._value);
    asn1::ber::EncoderOfSequence_T<4>::setField(2, _alrtPtrn);
  } else
    asn1::ber::EncoderOfSequence_T<4>::clearField(2);

  if (use_val._fieldsMask & use_val.has_msisdn) {
    _msIsdn.setValue(use_val.msisdn);
    asn1::ber::EncoderOfSequence_T<4>::setField(3, _msIsdn);
  } else
    asn1::ber::EncoderOfSequence_T<4>::clearField(3);
}

}}}}

