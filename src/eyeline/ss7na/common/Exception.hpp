#ifndef __EYELINE_SS7NA_COMMON_EXCEPTION_HPP__
# define __EYELINE_SS7NA_COMMON_EXCEPTION_HPP__

# include "util/Exception.hpp"
# include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class SCCPException : public smsc::util::Exception {
public:
  SCCPException(return_cause_value_t ret_cause_val)
  : Exception(), _retCauseVal(ret_cause_val)
  {}

  SCCPException(return_cause_value_t ret_cause_val, const char * fmt, ...)
  : Exception(), _retCauseVal(ret_cause_val)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }

  return_cause_value_t getFailureCode() const {
    return _retCauseVal;
  }

private:
  return_cause_value_t _retCauseVal;
};

class TranslationFailure : public SCCPException {
public:
  TranslationFailure(return_cause_value_t ret_cause_val, const char * fmt, ...)
  : SCCPException(ret_cause_val)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class RouteNotFound : public SCCPException {
public:
  RouteNotFound(return_cause_value_t ret_cause_val, const char * fmt, ...)
  : SCCPException(ret_cause_val)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class MTP3RouteNotFound : public smsc::util::Exception {
public:
  MTP3RouteNotFound(const char * fmt, ...)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
