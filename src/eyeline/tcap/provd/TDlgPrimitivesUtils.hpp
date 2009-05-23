/* ************************************************************************** *
 * Various utility functions concerning TCAP dialogue primitives processing.
 * ************************************************************************** */
#ifndef __ELC_TCAP_PRIMITIVES_UTILS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_PRIMITIVES_UTILS_HPP

#include "eyeline/sua/libsua/SuaApi.hpp"
#include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using sua::libsua::SuaApi;

extern TC_Notice_Ind::ReportCause_e 
  convertSuaApiError2TNoticeCause(SuaApi::ErrorCode_e sua_rc);

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_DISPATCHER_HPP */

