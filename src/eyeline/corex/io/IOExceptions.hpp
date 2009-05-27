#ifndef __EYELINE_COREX_IO_IOEXCEPTIONS_HPP__
# define __EYELINE_COREX_IO_IOEXCEPTIONS_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace corex {
namespace io {

class InputStream;
class OutputStream;

class IOException : public smsc::util::Exception {
public:
  IOException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
protected:
  IOException() {}
};

class EOFException : public IOException {
public:
  EOFException(const char * fmt, ...)
    : _sourceInputStream(NULL)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
  EOFException(const InputStream* sourceInputStream, const char * fmt, ...)
    : _sourceInputStream(sourceInputStream)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
  const InputStream* getSourceInputStream() const {
    return _sourceInputStream;
  }
private:
  const InputStream* _sourceInputStream;
};

class NotConnected : public IOException {
public:
  NotConnected(const char * fmt, ...)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class BrokenPipe : public IOException {
public:
  BrokenPipe(const char * fmt, ...)
    : _sourceOutputStream(NULL)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
  BrokenPipe(const OutputStream* sourceOutputStream, const char * fmt, ...)
    : _sourceOutputStream(sourceOutputStream)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
  const OutputStream* getSourceOutputStream() const {
    return _sourceOutputStream;
  }
private:
  const OutputStream* _sourceOutputStream;
};

class ConnectionFailedException : public IOException {
public:
  ConnectionFailedException(const char * fmt, ...)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
