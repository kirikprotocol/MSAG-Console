#ifndef __EYELINE_MAP_7F0_PROTO_ERR_CALLBARRINGCAUSE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_CALLBARRINGCAUSE_HPP__

# include <sys/types.h>

namespace eyeline {
namespace map {
namespace err {

struct CallBarringCause {
  enum NamedVal_e {
    bcBarringServiceActive = 0, bcOperatorBarring = 1
  };
  typedef uint8_t value_type;
  value_type value;

  //return true if ok, else false
  bool validate(value_type val) const {
    if (val == bcBarringServiceActive ||
        val == bcOperatorBarring)
      return true;
    return false;
  }
  bool validate() const {
    return validate(value);
  }
};

}}}

#endif
