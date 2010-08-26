#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/ROSPdu.hpp"

namespace eyeline {
namespace ros {
/* ************************************************************************* *
 * class ROSPduPrimitiveAC implementation
 * ************************************************************************* */
const char *
  ROSPduPrimitiveAC::nmKind(ROSPduPrimitiveAC::Kind_e ros_kind)
{
  switch (ros_kind) {
  case rosCancel:   return "rosCancel";
  case rosResult:   return "rosResult";
  case rosError:   return "rosError";
  case rosReject:   return "rosReject";
  case rosResultNL:   return "rosResultNL";
  default:;
  }
  return "rosUnknown";
}

} //ros
} //eyeline

