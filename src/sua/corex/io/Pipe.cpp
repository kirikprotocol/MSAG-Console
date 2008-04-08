#include <sua/corex/io/Pipe.hpp>
#include <unistd.h>
#include <stdio.h>

#include <logger/Logger.h>
namespace corex {
namespace io {

UnnamedPipe::UnnamedPipe()
{
  if ( ::pipe(_fds) < 0 )
    throw smsc::util::SystemError("UnnamedPipe::UnnamedPipe::: call to pipe() failed");
  _leftSide = new PipeEnd(this, _fds[0]);
  _rightSide = new PipeEnd(this, _fds[1]);
}

UnnamedPipe::~UnnamedPipe()
{ 
  close();
}

IOObject*
UnnamedPipe::getLeftSide()
{
  return _leftSide;
}

IOObject*
UnnamedPipe::getRightSide()
{
  return _rightSide;
}

void
UnnamedPipe::close()
{
  if ( _fds[0] > -1 ) {
    ::close(_fds[0]);
    _fds[0] = -1;
  }
  if ( _fds[1] > -1 ) {
    ::close(_fds[1]);
    _fds[1] = -1;
  }
  delete _leftSide; delete _rightSide; _leftSide = _rightSide = NULL;
}

std::string
UnnamedPipe::toString() const
{
  char str[64];
  snprintf(str, sizeof(str), "pipe:[infd=%d,outfd=%d]", _fds[0], _fds[1]);
  return std::string(str);
}

UnnamedPipe::PipeEnd::PipeEnd(UnnamedPipe* creator, int fd)
  : _creator(creator), _fd(fd), _inputStream(this, _fd), _outputStream(this, _fd)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("test");
  smsc_log_info(logger, "UnnamedPipe::PipeEnd::: _fd=%d", _fd);
}

UnnamedPipe::PipeEnd::~PipeEnd()
{
  close();
}

void
UnnamedPipe::PipeEnd::connect() {}

void
UnnamedPipe::PipeEnd::close()
{
  if ( _fd > -1 ) {
    _creator->halfClose(_fd);
    _fd = -1;
  }
}

void
UnnamedPipe::PipeEnd::setNonBlocking(bool on)
{
  IOObject::setNonBlocking(_fd, on);
}

InputStream*
UnnamedPipe::PipeEnd::getInputStream()
{
  return &_inputStream;
}

OutputStream*
UnnamedPipe::PipeEnd::getOutputStream()
{
  return &_outputStream;
}

std::string
UnnamedPipe::PipeEnd::toString() const
{
  char str[64];
  snprintf(str, sizeof(str), "pipe end:[fd=%d]", _fd);
  return std::string(str);
}

int
UnnamedPipe::PipeEnd::_getDescriptor()
{
  return _fd;
}

void
UnnamedPipe::halfClose(int fd)
{
  if ( fd == _fds[0] && fd > -1 ) {
    ::close(fd); _fds[0] = -1;
  } else if (  fd == _fds[1] && fd > -1 ) {
    ::close(fd); _fds[1] = -1;
  }
}

}}
