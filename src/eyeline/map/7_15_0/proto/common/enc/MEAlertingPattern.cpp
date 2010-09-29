#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/common/enc/MEAlertingPattern.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
   AlertingPattern ::= OCTET STRING (SIZE (1))
*/

void MEAlertingPattern::setValue(const AlertingPattern & use_val)
  /*throw(std::exception)*/
{
  asn1::ber::EncoderOfOCTSTR::setValue((asn1::TSLength)1, &use_val._value);
}

}}}}

