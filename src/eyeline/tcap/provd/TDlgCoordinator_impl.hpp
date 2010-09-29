/* ************************************************************************** *
 * TCAP Dialogue Coordinator:
 * ************************************************************************** */

template <class TDLG_REQ_COMPOSER>
TCRCode_e
  TDlgCoordinator::updateDlgByRequest(TDLG_REQ_COMPOSER & treq_comp, bool send_udt) /*throw()*/
{
  TDialogueId   dlgId = treq_comp.TReq().getDialogueId();
  TDlgRefPtr_t  pDlgFSM = _dlgReg->getTDlgFSM(dlgId);

  if (!pDlgFSM.get())
    return TCError::errDialogueId;

  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SCSPUnitdataReq   reqUDT(sccp::SCCPStandard::itut93);
  //check FSM state and complete request parameters(TransactionId, etc ...)
  TCRCode_e  rval = pDlgFSM->prepareDlgRequest(treq_comp, send_udt ? &reqUDT : NULL);
  if (rval != TCError::dlgOk)
    return rval;

  try { //switch FSM, activate timers if necessary
    rval = pDlgFSM->updateDlgByRequest(treq_comp);
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s: %s::update() exception=\'%s\'",
                   pDlgFSM->getLogId(), treq_comp.TReq().getIdent(), exc.what());
    rval = TCError::errTCAPUnknown;
  }
  if (send_udt && (rval == TCError::dlgOk))
    rval = notifyRmtTCUser(treq_comp.getTransactionId(), reqUDT, pDlgFSM->getSCSPLinkNum());
  return rval;
}

