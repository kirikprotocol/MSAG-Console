#include "TcpEstablishInd.hpp"

namespace eyeline {
namespace utilx {
namespace prot_fsm {

std::string
TcpEstablishInd::toString() const
{
  return "TCP_ESTABLISH_IND";
}

uint32_t
TcpEstablishInd::getIndicationTypeValue() const
{
  return TCP_ESTABLISH_IND;
}

}}}
