#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TDlgRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {

const char * 
  TDlgRequestPrimitive::getIdent(TDlgRequestPrimitive::RKind_e req_kind)
{
  switch (req_kind) {
  case TDlgRequestPrimitive::reqTRBegin:    return "TR_Begin_Req";
  case TDlgRequestPrimitive::reqTRCont:     return "TR_Cont_Req";
  case TDlgRequestPrimitive::reqTREnd:      return "TR_End_Req";
  case TDlgRequestPrimitive::reqTRUAbort:   return "TR_UAbort_Req";
  case TDlgRequestPrimitive::reqTRPAbort:   return "TR_PAbort_Req";
  default:;
  }
  return "undefined";
}


} //tcap
} //eyeline

