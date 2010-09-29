#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TComponentRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {

const char * 
  TComponentRequestPrimitive::getIdent(TComponentRequestPrimitive::RKind_e ind_kind)
{
  switch (ind_kind) {
  case TComponentRequestPrimitive::reqUCancel:      return "TC_U_CancelReq";
  case TComponentRequestPrimitive::reqTimerReset:   return "TC_TimerResetReq";
  default:;
  }
  return "undefinedReq";
}


} //tcap
} //eyeline

