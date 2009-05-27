#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/ContextlessOps.hpp"

#include "eyeline/tcap/provd/P_U_AbortPrimitiveUtils.hpp"
#include "eyeline/tcap/provd/TCAPLayer.hpp"
#include "eyeline/tcap/provd/TDlgIndComposers.hpp"
#include "eyeline/tcap/provd/TDlgReqComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void formPAbortRequest(const TDialogueId & dlg_id, PAbort::Cause_e p_abort_cause)
{
  TC_PAbort_Req pAbortReqPrimitive;
  pAbortReqPrimitive.setDialogueId(dlg_id);
  pAbortReqPrimitive.setAbortCause(p_abort_cause);
  TCAPLayer::getInstance().updateDialogue(pAbortReqPrimitive);
}

void formPAbortRequest(const proto::TransactionId& trn_id,
                       PAbort::Cause_e p_abort_cause,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr)
{
  TC_PAbort_Req pAbortReqPrimitive;
  pAbortReqPrimitive.setAbortCause(p_abort_cause);

  TPAbortReqComposer  reqComposer(pAbortReqPrimitive);
  reqComposer.setTransactionId(trn_id);

  TCAPLayer::getInstance().sendPrimitive(reqComposer, src_link_num, src_addr, dst_addr);
}

void formPAbortIndication(const TDialogueId& dlg_id,
                          PAbort::Cause_e p_abort_cause,
                          TDlgHandlerIface* t_dlg_hndlr_iface)
{
  try {
    TPAbortIndComposer pAbortIndPrimitive;
    pAbortIndPrimitive.setDialogueId(dlg_id);
    pAbortIndPrimitive.setPAbortCause(p_abort_cause);
    t_dlg_hndlr_iface->updateDialogue(pAbortIndPrimitive);
  } catch (...) {}
}

void formPAbortIndication(const TDialogueId& dlg_id,
                          PAbort::Cause_e p_abort_cause,
                          TDialogueServiceData* t_dlg_svc_data)
{
  try {
    TPAbortIndComposer pAbortIndPrimitive;
    pAbortIndPrimitive.setDialogueId(dlg_id);
    pAbortIndPrimitive.setPAbortCause(p_abort_cause);
    t_dlg_svc_data->updateDialogueDataByIndication(pAbortIndPrimitive);
  } catch (...) {}
}

void formUAbortRequest(const asn1::EncodedOID& app_ctx,
                       const proto::TransactionId& trn_id,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr)
{
  TC_UAbort_Req uAbortRequestPrimitive;
  if ( app_ctx == _ac_contextless_ops )
    uAbortRequestPrimitive.rejectDlgByUser(TDialogueAssociate::dsu_null);
  else {
    uAbortRequestPrimitive.rejectDlgByUser(TDialogueAssociate::dsu_ac_not_supported);
    uAbortRequestPrimitive.setAppCtx(app_ctx);
  }

  TUAbortReqComposer  reqComposer(uAbortRequestPrimitive);
  reqComposer.setTransactionId(trn_id);

  TCAPLayer::getInstance().sendPrimitive(reqComposer, src_link_num, src_addr, dst_addr);
}

}}}

