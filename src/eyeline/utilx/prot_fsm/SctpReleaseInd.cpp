#include "SctpReleaseInd.hpp"

namespace eyeline {
namespace utilx {
namespace prot_fsm {

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

}}}
