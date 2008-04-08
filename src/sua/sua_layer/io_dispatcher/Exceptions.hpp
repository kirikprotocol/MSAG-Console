#ifndef __IO_DISPATCHER_EXCEPTIONS_HPP__
# define __IO_DISPATCHER_EXCEPTIONS_HPP__ 1

# include <util/Exception.hpp>

namespace io_dispatcher {

class ProtocolException : public smsc::util::Exception {
public:
  ProtocolException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}

#endif
