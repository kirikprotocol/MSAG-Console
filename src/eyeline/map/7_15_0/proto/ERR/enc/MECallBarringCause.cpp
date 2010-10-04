#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void MECallBarringCause::setValue(const CallBarringCause & use_val)
{
  asn1::ber::EncoderOfENUM::setValue(use_val.value);
}

}}}}
