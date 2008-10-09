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

}}

#endif
