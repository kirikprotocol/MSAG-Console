#include "SctpEstablishInd.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

uint32_t
SctpEstablishInd::getIndicationTypeValue() const
{
  return SCTP_CONNECTION_IND;
}

std::string
SctpEstablishInd::toString() const
{
  return "SCTP_ESTABLISH_IND";
}

}}}}
