#ifndef __SUA_UTILX_EXCEPTION_HPP__
# define __SUA_UTILX_EXCEPTION_HPP__ 1

# include <util/Exception.hpp>

namespace utilx {

class FieldNotSetException : public smsc::util::Exception {
public:
  FieldNotSetException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class CongestionException : public smsc::util::Exception {
public:
  CongestionException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class InterruptedException : public smsc::util::Exception {
public:
  InterruptedException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}

#endif
