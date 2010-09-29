/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue requests handler interface.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLG_REQ_HANDLER_IFACE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDLG_REQ_HANDLER_IFACE_HPP

#include "eyeline/tcap/TcapErrors.hpp"
#include "eyeline/tcap/TDlgRequestPrimitives.hpp"
#include "eyeline/tcap/TComponentRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {

class TDlgReqHandlerIface {
protected:
  virtual ~TDlgReqHandlerIface() { }

public:
  // -- Dialogue(Component contained) handling request primitives handling
  virtual TCRCode_e updateDialogue(const TR_Begin_Req & tr_req) /*throw()*/= 0;
  virtual TCRCode_e updateDialogue(const TR_Cont_Req & tr_req) /*throw()*/= 0;
  virtual TCRCode_e updateDialogue(const TR_End_Req & tr_req) /*throw()*/= 0;
  virtual TCRCode_e updateDialogue(const TR_UAbort_Req & tr_req) /*throw()*/= 0;

  // -- Component only handling request primitives handling
  virtual TCRCode_e updateDialogue(const TC_U_Cancel_Req & tc_req) /*throw()*/= 0;
  virtual TCRCode_e updateDialogue(const TC_Timer_Reset_Req & tc_req) /*throw()*/= 0;
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDLG_REQ_HANDLER_IFACE_HPP */

