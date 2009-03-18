#include <stdio.h>
#include <util/Exception.hpp>

#include "TcpReleaseInd.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

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

}}}}
