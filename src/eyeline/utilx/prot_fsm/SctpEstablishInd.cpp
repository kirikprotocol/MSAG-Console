#include "SctpEstablishInd.hpp"

namespace eyeline {
namespace utilx {
namespace prot_fsm {

uint32_t
SctpEstablishInd::getIndicationTypeValue() const
{
  return SCTP_ESTABLISH_IND;
}

std::string
SctpEstablishInd::toString() const
{
  return "SCTP_ESTABLISH_IND";
}

}}}
