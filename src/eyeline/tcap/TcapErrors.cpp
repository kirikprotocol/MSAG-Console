#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/TcapErrors.hpp"

namespace eyeline {
namespace tcap {

//Returns string representation of ErrorCode_e value
const char * TCError::nmErrorCode(TCError::Code_e use_val)
{
  switch (use_val) {
  case TCError::errDialogueState:     return "errDialogueState";
  case TCError::errInvokeId:          return "errInvokeId";
  case TCError::errTooMuchComponents: return "errTooMuchComponents";
  case TCError::errComponentPortion:  return "errComponentPortion";
  case TCError::errDialoguePortion:   return "errDialoguePortion";
  case TCError::errDialogueId:        return "errDialogueId";
  case TCError::errDstAddress:        return "errDstAddress";
  case TCError::errSrcAddress:        return "errSrcAddress";
  case TCError::errSCCP:              return "errSCCP";
  case TCError::errTCAPLimit:         return "errTCAPLimit";
  case TCError::errTCAPUnknown:       return "errTCAPUnknown";
  case TCError::dlgOk:                return "dlgOk";
  default:;
  }
  return "undefinedErr";
}

} //tcap
} //eyeline

