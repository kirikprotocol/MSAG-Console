#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERREASON_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERREASON_HPP__

# include <inttypes.h>

namespace eyeline {
namespace map {
namespace err {

/*
 AbsentSubscriberReason ::= ENUMERATED {
        imsiDetach (0),
        restrictedArea (1),
        noPageResponse (2),
        ... ,
        purgedMS (3)}
*/
struct AbsentSubscriberReason {
  enum NamedVal_e {
    imsiDetach= 0,
    restrictedArea= 1,
    noPageResponse= 2,
    purgedMS= 3
  };
  typedef uint8_t value_type;
  // TODO: is need validate() ?
  value_type value;
};

}}}

#endif
