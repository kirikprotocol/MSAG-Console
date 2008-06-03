#ifndef __SUA_UTILX_EXCEPTION_HPP__
# define __SUA_UTILX_EXCEPTION_HPP__ 1

# include <util/Exception.hpp>
# include <sua/communication/types.hpp>

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

class TranslationFailure : public smsc::util::Exception {
public:
  TranslationFailure(communication::return_cause_value_t retCauseVal, const char * fmt, ...)
    : Exception(), _retCauseVal(retCauseVal)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }

  communication::return_cause_value_t getFailureCode() const {
    return _retCauseVal;
  }
private:
  communication::return_cause_value_t _retCauseVal;
};

}

#endif
