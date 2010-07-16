#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-Res.hpp"

namespace eyeline {
namespace map {
namespace ss {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
 USSD-Res ::= SEQUENCE {
        ussd-DataCodingScheme   USSD-DataCodingScheme,
        ussd-String             USSD-String,
        ...
} */

//inits mandatory fields encoders
void MEUSSD_Res::construct(void)
{
  asn1::ber::EncoderOfSequence_T<2>::setField(0, _dcs);
  asn1::ber::EncoderOfSequence_T<2>::setField(1, _ussd);
}

//
void MEUSSD_Res::setValue(const USSD_Res & use_val) /*throw(std::exception)*/
{
  _dcs.setValue(use_val.ussd_DataCodingScheme);
  _ussd.setValue(use_val.ussd_String);
}

}}}}

