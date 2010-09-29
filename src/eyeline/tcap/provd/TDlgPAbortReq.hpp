/* ************************************************************************** *
 * structured TCAP dialogue requests primitives(transaction sublayer). 
 * ************************************************************************** */
#ifndef __ELC_TCAP_PABORT_REQUEST_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_PABORT_REQUEST_HPP

#include "eyeline/tcap/TDlgRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::PAbort;
//
class TR_PAbort_Req : public eyeline::tcap::TDlgRequestPrimitive {
protected:
  PAbort::Cause_e _cause;

public:
  TR_PAbort_Req() : TDlgRequestPrimitive(reqTRPAbort)
  { }
  ~TR_PAbort_Req()
  { }

  PAbort::Cause_e getAbortCause(void) const { return _cause; }
  void setAbortCause(PAbort::Cause_e use_reason) { _cause = use_reason; }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_PABORT_REQUEST_HPP */

