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
        // ... unknwon extensions
} */

//inits mandatory fields encoders
void MEUSSD_Arg::construct(void)
{
  setField(0, _dcs);
  setField(1, _ussd);
}

//
void MEUSSD_Arg::setValue(const USSD_Arg & use_val) /*throw(std::exception)*/
{
  _dcs.setValue(use_val.ussd_DataCodingScheme);
  _ussd.setValue(use_val.ussd_String);
  //clear optionals and unknown extensions first
  clearFields(2);

  if (use_val.alertingPattern.get()) {
    _eAlrtPtrn.init(getTSRule()).setValue(*use_val.alertingPattern.get());
    setField(2, *_eAlrtPtrn.get());
  }
  if (use_val.msisdn.get()) {
    _eMsIsdn.init(_tag_f3, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(*use_val.msisdn.get());
    setField(3, *_eMsIsdn.get());
  }
  if (!use_val._unkExt._tsList.empty())
    setExtensions(use_val._unkExt, 4);
}

}}}}

