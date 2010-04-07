#ifndef __EYELINE_SS7NA_LIBSCCP_EXCEPTION_HPP__
# define __EYELINE_SS7NA_LIBSCCP_EXCEPTION_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class SccpLibException : public smsc::util::Exception {
public:
  SccpLibException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
