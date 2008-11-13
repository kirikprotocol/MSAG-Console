#ifndef __SMSC_MCISME_EXCEPTIONS_HPP__
# define __SMSC_MCISME_EXCEPTIONS_HPP__

# include <util/Exception.hpp>

namespace smsc {
namespace mcisme {

class NetworkException : public util::Exception {
public:
  NetworkException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
protected:
  NetworkException() {}
};

class TimeoutException : public NetworkException {
public:
  TimeoutException(const char * fmt, ...)
    : NetworkException()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class UnrecoveredProtocolError : public util::Exception {
public:
};

}}

#endif
