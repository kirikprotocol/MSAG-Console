#include <sua/corex/io/IOObject.hpp>
#include <util/Exception.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

namespace corex {
namespace io {

int _turn_Off_SIGPIPE_Delivery()
{
  sigset(SIGPIPE, SIG_IGN);
  return 0;
}

static int turn_Off_SIGPIPE_Delivery = _turn_Off_SIGPIPE_Delivery();

void
IOObject::setNonBlocking(int fd, bool on)
{
  int flags;
  if ( fcntl(fd, F_GETFL, &flags) < 0 )
    throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_GETFL) failed");
  if ( on ) {
    flags |= O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, &flags) < 0 )
      throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_SETFL) failed");
  } else {
    flags &= ~O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, &flags) < 0 )
      throw smsc::util::SystemError("IOObject::setNonBlocking::: call to fcntl(F_SETFL) failed");
  }
}

int
IOObject::getDescriptor()
{
  return _getDescriptor();
}

}}
