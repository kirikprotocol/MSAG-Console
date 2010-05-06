#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_EXCEPTION_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_EXCEPTION_HPP__

# include "util/Exception.hpp"
# include "eyeline/utilx/runtime_cfg/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
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

inline
void
generateExceptionAndForcePopUpCurrentInterpreter(const std::string& message_to_user,
                                                 const char * fmt, ...)
{
  va_list arglist;
  va_start(arglist,fmt);
  std::string message;
  smsc::util::vformat(message, fmt, arglist);
  va_end(arglist);

  utilx::runtime_cfg::InconsistentConfigCommandException generatedException(message_to_user, message.c_str());
  generatedException.forcePopUpCurrentCommandInterpreter();
  throw generatedException;
}

}}}}

#endif
