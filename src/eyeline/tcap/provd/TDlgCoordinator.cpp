#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPConsts.hpp"
#include "eyeline/tcap/provd/TDlgCoordinator.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

// --------------------------------------------------------------
// -- TDlgCoordinatorIface interface methods implementation
// --------------------------------------------------------------
//
TCRCode_e
  TDlgCoordinator::notifyLclTCUser(const TransactionId & tr_id,
                                  const SCSPUnitdataReq & req_udt) /*throw()*/
{
  TIndLclNotice * pInd = _indGenerator->allcTNoticeInd();

  if (!pInd) {
    smsc_log_fatal(_logger, "Trn[%s]: unable to notify TCUser - indications task limit reached",
                   tr_id.toString().c_str());
    return TCError::errTCAPLimit;
  }

  pInd->setDialogueId(tr_id.getIdLocal());
  pInd->setReturnCause(sccp::ReturnCause::rcLocalProcessing);
  pInd->setOrigAddress(req_udt.callingAddr(), req_udt.callingAddrLen());
  pInd->setDestAddress(req_udt.calledAddr(), req_udt.calledAddrLen());
  pInd->setDataBuffer(req_udt.dataBuf()); //copy user data
  /**/
  pInd->process();
  return TCError::dlgOk;
}
//
TCRCode_e
  TDlgCoordinator::notifyLclTCUser(const TransactionId & tr_id, PAbort::Cause_e abrt_cause,
                                   unsigned int scsp_link_num) /*throw()*/
{
  TIndLclPAbort * pInd = _indGenerator->allcPAbortInd();

  if (!pInd) {
    smsc_log_fatal(_logger, "Trn[%s]: unable to notify TCUser - indications task limit reached",
                   tr_id.toString().c_str());
    return TCError::errTCAPLimit;
  }

  pInd->init(tr_id.getIdRemote(), abrt_cause);
  pInd->setSCSPLink(scsp_link_num);
  /**/
  pInd->process();
  return TCError::dlgOk;
}

TCRCode_e
  TDlgCoordinator::notifyLclTCUser(const TransactionId & tr_id, uint8_t inv_id) /*throw()*/
{
  TCIndLclCancel * pInd = _indGenerator->allcLCancelInd();

  if (!pInd) {
    smsc_log_fatal(_logger, "Trn[%s]: unable to notify TCUser - indications task limit reached",
                   tr_id.toString().c_str());
    return TCError::errTCAPLimit;
  }

  pInd->setDialogueId(tr_id.getIdLocal());
  pInd->setInvokeId(inv_id);
  /**/
  pInd->process();
  return TCError::dlgOk;
}

//
TCRCode_e
  TDlgCoordinator::notifyRmtTCUser(const TransactionId & tr_id,
                                   SCSPUnitdataReq req_udt, unsigned int scsp_link_num)
  /*throw()*/
{
  SccpApi::ErrorCode_e
    rc = _sccpApi->unitdata_req(req_udt.userData(), req_udt.userDataLen(),
                                req_udt.calledAddr(), req_udt.calledAddrLen(),
                                req_udt.callingAddr(), req_udt.callingAddrLen(),
                                req_udt, scsp_link_num);

  if (rc != SccpApi::OK) {
    smsc_log_error(_logger, "Trn[%s]: sendUDT() error=[%d]",
                   tr_id.toString().c_str(), rc);
    if (req_udt.getReturnOnError())
      notifyLclTCUser(tr_id, req_udt);
    return TCError::errSCCP;
  }
  return TCError::dlgOk;
}

//
TCRCode_e
  TDlgCoordinator::notifyRmtTCUser(const TransactionId & tr_id,
                                   PAbort::Cause_e abrt_cause, unsigned int scsp_link_num,
                                   const SCCPAddress & src_addr, const SCCPAddress & dst_addr)
  /*throw()*/
{
  TR_PAbort_Req       trReq;
  TPAbortReqComposer  reqComp(trReq);

  trReq.setDialogueId(tr_id.getIdLocal());
  trReq.setReturnOnError(false);
  trReq.setInSequenceDelivery(false);
  trReq.setAbortCause(abrt_cause);
  reqComp.setTransactionId(tr_id);

  TCRCode_e  rval = TCError::dlgOk;
  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SCSPUnitdataReq   reqUDT(sccp::SCCPStandard::itut93);

  try {
    rval = reqComp.serialize2UDT(reqUDT, src_addr, dst_addr);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "Trn[%s]: %s::serialize2UDT() exception=\'%s\'",
                   tr_id.toString().c_str(), trReq.getIdent(), exc.what());
    rval = TCError::errTCAPUnknown;
  }
  return (rval == TCError::dlgOk) ? notifyRmtTCUser(tr_id, reqUDT, scsp_link_num) : rval;
}


TCRCode_e
  TDlgCoordinator::notifyRmtTCUser(const TransactionId & tr_id, 
                                    TDialogueAssociate::DiagnosticUser_e abrt_cause,
                                    unsigned int scsp_link_num,
                                    const SCCPAddress & src_addr, const SCCPAddress & dst_addr)
  /*throw()*/
{
  TR_UAbort_Req       trReq;
  TUAbortReqComposer  reqComp(trReq);

  trReq.setDialogueId(tr_id.getIdLocal());
  trReq.setReturnOnError(false);
  trReq.setInSequenceDelivery(false);
  trReq.abortAssociation(abrt_cause);
  reqComp.setTransactionId(tr_id);

  TCRCode_e  rval = TCError::dlgOk;
  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SCSPUnitdataReq   reqUDT(sccp::SCCPStandard::itut93);

  try {
    rval = reqComp.serialize2UDT(reqUDT, src_addr, dst_addr);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "Trn[%s]: %s::serialize2UDT() exception=\'%s\'",
                   tr_id.toString().c_str(), trReq.getIdent(), exc.what());
    rval = TCError::errTCAPUnknown;
  }
  return (rval == TCError::dlgOk) ? notifyRmtTCUser(tr_id, reqUDT, scsp_link_num) : rval;
}

void TDlgCoordinator::rlseDialogue(TDialogueId dlg_id) /*throw()*/
{
  _dlgReg->releaseTDlgFSM(dlg_id);
}


// ---------------------------------------------------------
// -- TDlgReqHandlerIface interface methods implementation
// ---------------------------------------------------------

TCRCode_e
  TDlgCoordinator::updateDialogue(const TR_Begin_Req & tr_req) /*throw()*/
{
  TBeginReqComposer     tReqComposer(tr_req);
  return updateDlgByRequest(tReqComposer, true);
}

TCRCode_e
  TDlgCoordinator::updateDialogue(const TR_Cont_Req & tr_req) /*throw()*/
{
  TContReqComposer tReqComposer(tr_req);
  return updateDlgByRequest(tReqComposer, true);
}

TCRCode_e
  TDlgCoordinator::updateDialogue(const TR_End_Req & tr_req) /*throw()*/
{
  TEndReqComposer tReqComposer(tr_req);
  return updateDlgByRequest(tReqComposer, (tr_req.getTermination() == TR_End_Req::endBASIC));
}

TCRCode_e
  TDlgCoordinator::updateDialogue(const TR_UAbort_Req & tr_req) /*throw()*/
{
  TUAbortReqComposer tReqComposer(tr_req);
  return updateDlgByRequest(tReqComposer, true);
}

//TCRCode_e
//  TDlgCoordinator::updateDialogue(const TR_PAbort_Req & tr_req) /*throw()*/
/*{
  TPAbortReqComposer tReqComposer(tr_req);
  return updateDlgByRequest(tReqComposer, true);
}*/


// -- Component only handling request primitives handling
TCRCode_e
  TDlgCoordinator::updateDialogue(const TC_U_Cancel_Req & tc_req) /*throw()*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgReg->getTDlgFSM(tc_req.getDialogueId());
  return pDlgFSM.get() ? pDlgFSM->updateDlgByRequest(tc_req)
                        : TCError::errDialogueId;
}

TCRCode_e
  TDlgCoordinator::updateDialogue(const TC_Timer_Reset_Req & tc_req) /*throw()*/
{
  TDlgRefPtr_t  pDlgFSM = _dlgReg->getTDlgFSM(tc_req.getDialogueId());
  return pDlgFSM.get() ? pDlgFSM->updateDlgByRequest(tc_req)
                        : TCError::errDialogueId;
}

}}}

