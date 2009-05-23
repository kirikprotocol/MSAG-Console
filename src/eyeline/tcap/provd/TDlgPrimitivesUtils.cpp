#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgPrimitivesUtils.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TC_Notice_Ind::ReportCause_e 
convertSuaApiError2TNoticeCause(SuaApi::ErrorCode_e sua_rc)
{
  switch (sua_rc) {
  case SuaApi::OK:
  case SuaApi::ALREADY_BINDED: return TC_Notice_Ind::errOk;
  /* */
  case SuaApi::SUA_NOT_INITIALIZED:
  case SuaApi::NOT_CONNECTED:
  case SuaApi::NOT_BINDED: return TC_Notice_Ind::errUserUneqipped;
  /* */
  case SuaApi::SOCKET_TIMEOUT:  return TC_Notice_Ind::errNetworkFailure;
  /* */
  case SuaApi::BIND_CONFIRM_UNKNOWN_APP_ID_VALUE:
  case SuaApi::BIND_CONFIRM_UNSUPPORTED_PROTOCOL_VERSION:
  case SuaApi::BIND_CONFIRM_COMPONENT_IS_INACTIVE:  //return TC_Notice_Ind::errSCCPFailure;
  /* */
  case SuaApi::WRONG_CONNECT_NUM:
  case SuaApi::GOT_TOO_LONG_MESSAGE: //return TC_Notice_Ind::errSCCPFailure;
  /* */
  case SuaApi::SYSTEM_MALFUNCTION:
  case SuaApi::SYSTEM_ERROR:        //return TC_Notice_Ind::errSCCPFailure;
  default:;
  }
  return eyeline::tcap::TC_Notice_Ind::errSCCPFailure;
}

} //provd
} //tcap
} //eyeline

