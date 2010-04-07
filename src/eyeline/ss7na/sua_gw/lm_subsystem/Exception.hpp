#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_EXCEPTION_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_EXCEPTION_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class InvalidCommandLineException : public smsc::util::Exception {
public:
  InvalidCommandLineException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class UserTerminateSessionException : public smsc::util::Exception {
public:
};

}}}}

#endif
