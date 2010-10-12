#include "MDCallBarringCause.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDCallBarringCause::setValue(CallBarringCause & value) {
  asn1::ber::DecoderOfINTEGER::setValue(value.value);
}

}}}}
