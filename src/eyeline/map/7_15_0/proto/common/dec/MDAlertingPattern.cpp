#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/common/dec/MDAlertingPattern.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
   AlertingPattern�::= OCTET STRING (SIZE (1))
*/


void MDAlertingPattern::setValue(AlertingPattern::value_type & use_val)
  /*throw(std::exception)*/
{
  _outVal.assign(1, &use_val, 0);
  asn1::ber::DecoderOfOCTSTRTiny::setValue(_outVal, 1);
}

}}}}

