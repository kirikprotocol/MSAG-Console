#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TComponentIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {

const char * 
  TComponentIndicationPrimitive::getIdent(TComponentIndicationPrimitive::IKind_e ind_kind)
{
  switch (ind_kind) {
  case TComponentIndicationPrimitive::indLCancel:    return "TC_L_CancelInd";
  case TComponentIndicationPrimitive::indLReject:    return "TC_L_RejectInd";
  default:;
  }
  return "undefinedInd";
}


} //tcap
} //eyeline

