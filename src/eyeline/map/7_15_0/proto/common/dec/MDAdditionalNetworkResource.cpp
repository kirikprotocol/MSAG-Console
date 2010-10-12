#include "MDAdditionalNetworkResource.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

void
MDAdditionalNetworkResource::setValue(AdditionalNetworkResource & value)
{
  asn1::ber::DecoderOfENUM::setValue(value._value);
}

}}}}
