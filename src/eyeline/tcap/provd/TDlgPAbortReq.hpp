/* ************************************************************************** *
 * TCAP dialogue requests senders (serialization and sending).
 * ************************************************************************** */
#ifndef __ELC_TCAP_PABORT_REQUEST_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_PABORT_REQUEST_HPP

#include "eyeline/tcap/TDialogueRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::PAbort;
//
class TC_PAbort_Req : public eyeline::tcap::TDialogueRequestPrimitive {
protected:
  PAbort::Cause_e _cause;

public:
  TC_PAbort_Req()
    : TDialogueRequestPrimitive()
  { }

  PAbort::Cause_e getAbortCause(void) const { return _cause; }
  void setAbortCause(PAbort::Cause_e use_reason) { _cause = use_reason; }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_PABORT_REQUEST_HPP */

