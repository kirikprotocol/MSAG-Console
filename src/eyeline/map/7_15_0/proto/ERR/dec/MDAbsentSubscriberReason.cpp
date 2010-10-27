#include "MDAbsentSubscriberReason.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDAbsentSubscriberReason::setValue(AbsentSubscriberReason & value)
{
  asn1::ber::DecoderOfINTEGER::setValue(value.value);
}

}}}}
