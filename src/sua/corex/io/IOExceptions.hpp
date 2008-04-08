#ifndef __COREX_IO_IOEXCEPTIONS_HPP__
# define __COREX_IO_IOEXCEPTIONS_HPP__ 1

# include <util/Exception.hpp>

namespace corex {
namespace io {

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
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
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
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class ProtocolException : public IOException {
public:
  ProtocolException(const char * fmt, ...)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}

#endif
