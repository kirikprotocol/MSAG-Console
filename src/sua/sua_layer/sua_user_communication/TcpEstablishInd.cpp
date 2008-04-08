#include "TcpEstablishInd.hpp"
#include <stdio.h>
#include <util/Exception.hpp>

namespace sua_user_communication {

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

}
