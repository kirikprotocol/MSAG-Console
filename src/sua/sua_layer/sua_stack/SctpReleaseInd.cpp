#include "SctpReleaseInd.hpp"

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

}
