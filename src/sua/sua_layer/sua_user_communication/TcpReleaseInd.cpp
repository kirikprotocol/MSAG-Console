#include "TcpReleaseInd.hpp"
#include <stdio.h>
#include <util/Exception.hpp>

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

}
