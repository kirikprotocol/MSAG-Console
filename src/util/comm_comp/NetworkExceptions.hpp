#ifndef __SMSC_UTIL_COMM_COMP_NETWORKEXCEPTIONS_HPP__
# define __SMSC_UTIL_COMM_COMP_NETWORKEXCEPTIONS_HPP__ 1

# include <util/Exception.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

class EOFException : public smsc::util::Exception {
public:
  EOFException() : Exception() {};
  EOFException(const char* fmt, ...)
    : Exception() { SMSC_UTIL_EX_FILL(fmt); };
  virtual ~EOFException() throw() {};
};

class SocketException : public smsc::util::Exception {
public:
  SocketException() : Exception() {};
  SocketException(const char* fmt, ...)
    : Exception() { SMSC_UTIL_EX_FILL(fmt); };
  virtual ~SocketException() throw() {};
};
}}}

#endif
