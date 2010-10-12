#include "MDNetworkResource.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

void
MDNetworkResource::setValue(NetworkResource &value)
{
  asn1::ber::DecoderOfENUM::setValue(value._value);
}

}}}}
