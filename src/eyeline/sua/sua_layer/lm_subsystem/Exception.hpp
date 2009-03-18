#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_EXCEPTION_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_EXCEPTION_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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
