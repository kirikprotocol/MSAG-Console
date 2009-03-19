#ifndef __EYELINE_SUA_LIBSUA_EXCEPTION_HPP__
# define __EYELINE_SUA_LIBSUA_EXCEPTION_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class SuaLibException : public smsc::util::Exception {
public:
  SuaLibException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
