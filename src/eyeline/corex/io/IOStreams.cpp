#include <eyeline/corex/io/IOStreams.hpp>
#include <eyeline/corex/io/IOExceptions.hpp>
#include <unistd.h>

namespace eyeline {
namespace corex {
namespace io {

GenericInputStream::GenericInputStream(IOObject* owner, int fd) : _owner(owner), _fd(fd) {}

ssize_t
GenericInputStream::read(uint8_t *buf, size_t bufSz)
{
  ssize_t result = ::read(_fd, buf, bufSz);
  if ( result < 0 )
    throw smsc::util::SystemError("GenericInputStream::read::: call to read() failed");
  if ( !result )
    throw EOFException("GenericInputStream::read::: connection closed by remote side");

  return result;
}

IOObject*
GenericInputStream::getOwner() { return _owner; }

GenericOutputStream::GenericOutputStream(IOObject* owner, int fd) : _owner(owner), _fd(fd) {}

ssize_t
GenericOutputStream::write(const uint8_t *buf, size_t bufSz, uint16_t streamNo, bool ordered) const
{
  ssize_t result = ::write(_fd, buf, bufSz);
  if ( result < 0 ) {
    if ( errno == EPIPE )
      throw corex::io::BrokenPipe("GenericOutputStream::write::: write() returned EPIPE");
    else
      throw smsc::util::SystemError("GenericOutputStream::write::: call to write() failed");
  }
  return result;
}

ssize_t
GenericOutputStream::writev(const struct iovec *iov, int iovcnt) const
{
  ssize_t result = ::writev(_fd, iov, iovcnt);
  if ( result < 0 ) {
    if ( errno == EPIPE )
      throw corex::io::BrokenPipe("GenericOutputStream::writev::: write() returned EPIPE");
    else
      throw smsc::util::SystemError("GenericOutputStream::writev::: call to write() failed");
  }
  return result;
}

IOObject*
GenericOutputStream::getOwner() { return _owner; }

}}}
