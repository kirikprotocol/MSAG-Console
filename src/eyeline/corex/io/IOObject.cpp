#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "util/Exception.hpp"
#include "eyeline/corex/io/IOObject.hpp"

namespace eyeline {
namespace corex {
namespace io {

namespace{
struct turn_Off_SIGPIPE_Delivery_Struct
{
  turn_Off_SIGPIPE_Delivery_Struct()
  {
    sigset(SIGPIPE, SIG_IGN);
  }
} turn_Off_SIGPIPE_Delivery;
}

//static int turn_Off_SIGPIPE_Delivery = _turn_Off_SIGPIPE_Delivery();

void
IOObject::setNonBlocking(int fd, bool on)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if ( flags < 0 )
    throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_GETFL) failed");
  if ( on ) {
    flags |= O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, flags) < 0 )
      throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_SETFL) failed");
  } else {
    flags &= ~O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, flags) < 0 )
      throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_SETFL) failed");
  }
}

int
IOObject::getDescriptor()
{
  return _getDescriptor();
}

const std::string&
IOObject::getId() const
{
  return _id;
}

void
IOObject::setId(const std::string& id)
{
  _id = id;
}

}}}
