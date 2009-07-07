#include "TcpReleaseInd.hpp"

namespace eyeline {
namespace utilx {
namespace prot_fsm {

std::string
TcpReleaseInd::toString() const
{
  return "TCP_RELEASE_IND";
}

uint32_t
TcpReleaseInd::getIndicationTypeValue() const
{
  return TCP_RELEASE_IND;
}

}}}
