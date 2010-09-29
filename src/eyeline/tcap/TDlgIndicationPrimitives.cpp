#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TDlgIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {

const char * 
  TDlgIndicationPrimitive::getIdent(TDlgIndicationPrimitive::IKind_e ind_kind)
{
  switch (ind_kind) {
  case TDlgIndicationPrimitive::indTRBegin:    return "TR_Begin_Ind";
  case TDlgIndicationPrimitive::indTRCont:     return "TR_Cont_Ind";
  case TDlgIndicationPrimitive::indTREnd:      return "TR_End_Ind";
  case TDlgIndicationPrimitive::indTRUAbort:   return "TR_UAbort_Ind";
  case TDlgIndicationPrimitive::indTRPAbort:   return "TR_PAbort_Ind";
  case TDlgIndicationPrimitive::indTRNotice:   return "TR_Notice_Ind";
  default:;
  }
  return "undefined";
}


} //tcap
} //eyeline

