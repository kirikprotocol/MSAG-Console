#include "SctpReleaseInd.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

uint32_t
SctpReleaseInd::getIndicationTypeValue() const
{
  return SCTP_RELEASE_IND;
}

std::string
SctpReleaseInd::toString() const
{
  return "SCTP_RELEASE_IND";
}

}}}}
