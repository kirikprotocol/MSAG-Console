#ifndef __EYELINE_UTILX_EXCEPTION_HPP__
# define __EYELINE_UTILX_EXCEPTION_HPP__

# include <util/Exception.hpp>

namespace eyeline {
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

class UnsupportedOperationException : public smsc::util::Exception {
public:
  UnsupportedOperationException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}

#endif
