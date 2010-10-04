#ifndef __EYELINE_MAP_7F0_PROTO_ERR_FAILURECAUSEPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_FAILURECAUSEPARAM_HPP__

#include <inttypes.h>

namespace eyeline {
namespace map {
namespace err {

/*
  FailureCauseParam ::= ENUMERATED {
        limitReachedOnNumberOfConcurrentLocationRequests (0),
        ... }
  -- if unknown value is received in FailureCauseParam it shall be ignored
 */
struct FailureCauseParam {
  enum NamedVal_e {
    limitReachedOnNumberOfConcurrentLocationRequests = 0
  };
  typedef uint8_t value_type;
  value_type value;

  //return true if ok, else false
  bool validate(value_type val) const {
    return (val == limitReachedOnNumberOfConcurrentLocationRequests);
  }
  bool validate() const {
    return validate(value);
  }
};

}}}

#endif
