#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TcapApi.hpp"

namespace eyeline {
namespace tcap {

//Returns string representation of ErrorCode_e value
const char * TcapApi::nmErrorCode(ErrorCode_e use_val)
{
  switch (use_val) {
  case TcapApi::errInvokeId:          return "errInvokeId";
  case TcapApi::errTooMuchComponents: return "errTooMuchComponents";
  case TcapApi::errComponentPortion:  return "errComponentPortion";
  case TcapApi::errDialoguePortion:   return "errDialoguePortion";
  case TcapApi::errDialogueId:        return "errDialogueId";
  case TcapApi::errDstAddress:        return "errDstAddress";
  case TcapApi::errSrcAddress:        return "errSrcAddress";
  case TcapApi::errSCCP:              return "errSCCP";
  case TcapApi::errTCAP:              return "errTCAP";
  case TcapApi::dlgOk:                return "dlgOk";
  default:;
  }
  return "undefined";
}


} //tcap
} //eyeline

