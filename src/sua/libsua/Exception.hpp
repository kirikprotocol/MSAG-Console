#ifndef __SUA_LIBSUA_EXCEPTION_HPP__
# define __SUA_LIBSUA_EXCEPTION_HPP__ 1

# include <util/Exception.hpp>

namespace libsua {

class SuaLibException : public smsc::util::Exception {
public:
  SuaLibException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}

#endif
