#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_EXCEPTIONS_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_EXCEPTIONS_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class ProtocolException : public smsc::util::Exception {
public:
  ProtocolException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}}

#endif
