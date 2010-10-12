#include "MDFailureCauseParam.hpp"

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
