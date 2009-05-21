#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/ContextlessOps.hpp"
#include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"

#include "eyeline/tcap/provd/P_U_AbortPrimitiveUtils.hpp"
#include "eyeline/tcap/provd/TDlgPAbortReq.hpp"
#include "eyeline/tcap/provd/TCAPLayer.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void formPAbortRequest(const proto::TransactionId& trnId,
                       TDialogueHandlingPrimitive::PAbortCause_e pAbortCause)
{
  TC_PAbort_Req pAbortReqPrimitive;
  pAbortReqPrimitive.setTransactionId(trnId);
  pAbortReqPrimitive.setAbortCause(pAbortCause);
  TCAPLayer::getInstance().updateDialogue(&pAbortReqPrimitive);
}

void formPAbortRequest(const proto::TransactionId& trn_id,
                       TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr)
{
  TC_PAbort_Req pAbortReqPrimitive;
  pAbortReqPrimitive.setTransactionId(trn_id);
  pAbortReqPrimitive.setAbortCause(p_abort_cause);
  TCAPLayer::getInstance().sendPrimitive(&pAbortReqPrimitive, src_link_num, src_addr, dst_addr);
}

void formPAbortIndication(const TDialogueId& dlg_id,
                          TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                          TDlgHandlerIface* t_dlg_hndlr_iface)
{
  try {
    TC_PAbort_Ind pAbortIndPrimitive;
    pAbortIndPrimitive.setDialogueId(dlg_id);
    pAbortIndPrimitive.setPAbortCause(p_abort_cause);
    t_dlg_hndlr_iface->updateDialogue(pAbortIndPrimitive);
  } catch (...) {}
}

void formUAbortRequest(const asn1::EncodedOID& app_ctx,
                       const proto::TransactionId& trn_id,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr)
{
  TC_UAbort_Req uAbortRequestPrimitive;
  uAbortRequestPrimitive.setTransactionId(trn_id);
  if ( app_ctx == _ac_contextless_ops )
    uAbortRequestPrimitive.rejectDlgByUser(proto::AssociateSourceDiagnostic::dsu_null);
  else {
    uAbortRequestPrimitive.rejectDlgByUser(proto::AssociateSourceDiagnostic::dsu_ac_not_supported);
    uAbortRequestPrimitive.setAppCtx(app_ctx);
  }

  TCAPLayer::getInstance().sendPrimitive(&uAbortRequestPrimitive, src_link_num, src_addr, dst_addr);
}

}}}

