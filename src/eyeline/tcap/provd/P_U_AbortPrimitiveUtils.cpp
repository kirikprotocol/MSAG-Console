#include "P_U_AbortPrimitiveUtils.hpp"
#include "TDlgReqSerializer.hpp"
#include "TCAPLayer.hpp"
#include "eyeline/tcap/ContextlessOps.hpp"
#include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"

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

void formPAbortRequest(const proto::TransactionId& trnId,
                       TDialogueHandlingPrimitive::PAbortCause_e pAbortCause,
                       unsigned int srcLinkNum)
{
  TC_PAbort_Req pAbortReqPrimitive;
  pAbortReqPrimitive.setTransactionId(trnId);
  pAbortReqPrimitive.setAbortCause(pAbortCause);
  TCAPLayer::getInstance().updateDialogue(&pAbortReqPrimitive, srcLinkNum);
}

void formPAbortIndication(const TDialogueId& dlgId,
                          TDialogueHandlingPrimitive::PAbortCause_e pAbortCause,
                          TDlgHandlerIface* tDlgHndlrIface)
{
  try {
    TC_PAbort_Ind pAbortIndPrimitive;
    pAbortIndPrimitive.setDialogueId(dlgId);
    pAbortIndPrimitive.setPAbortCause(pAbortCause);
    tDlgHndlrIface->updateDialogue(pAbortIndPrimitive);
  } catch (...) {}
}

void formUAbortRequest(const asn1::EncodedOID& appCtx,
                       const proto::TransactionId& trnId)
{
  TC_UAbort_Req uAbortRequestPrimitive;
  uAbortRequestPrimitive.setTransactionId(trnId);
  if ( appCtx == _ac_contextless_ops )
    uAbortRequestPrimitive.rejectDlgByUser(proto::AssociateSourceDiagnostic::dsu_null);
  else {
    uAbortRequestPrimitive.rejectDlgByUser(proto::AssociateSourceDiagnostic::dsu_ac_not_supported);
    uAbortRequestPrimitive.setAppCtx(appCtx);
  }

  TCAPLayer::getInstance().updateDialogue(&uAbortRequestPrimitive);
}

}}}

