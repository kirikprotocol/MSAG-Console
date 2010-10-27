#include "MEAbsentSubscriberReason.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MEAbsentSubscriberReason::setValue(const AbsentSubscriberReason & use_val)
{
  asn1::ber::EncoderOfENUM::setValue(use_val.value);
}

}}}}
