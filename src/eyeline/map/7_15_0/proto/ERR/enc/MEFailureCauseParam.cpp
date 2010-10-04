#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MEFailureCauseParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void MEFailureCauseParam::setValue(const FailureCauseParam & use_val)
{
  asn1::ber::EncoderOfENUM::setValue(use_val.value);
}

}}}}
