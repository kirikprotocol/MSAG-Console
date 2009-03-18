#include "ServerSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

int
ServerSocket::getDescriptor()
{
  return _getDescriptor();
}

}}}}
