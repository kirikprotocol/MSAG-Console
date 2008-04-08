#include "ServerSocket.hpp"

namespace corex {
namespace io {
namespace network {

int
ServerSocket::getDescriptor()
{
  return _getDescriptor();
}

}}}
