#include "MDFailureCauseParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDFailureCauseParam::setValue(FailureCauseParam & value)
{
  asn1::ber::DecoderOfENUM::setValue(value.value);
}

}}}}
