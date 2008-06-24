#ifndef __SUA_SUALAYER_LMSUBSYSTEM_EXCEPTION_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_EXCEPTION_HPP__ 1

# include <util/Exception.hpp>

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

}

#endif
