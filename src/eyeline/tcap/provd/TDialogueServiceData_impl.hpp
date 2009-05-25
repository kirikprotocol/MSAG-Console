
template<class T_DIALOGUE_TERMINATION_IND_PRIMITIVE>
void
TDialogueServiceData::handleDialogueTerminationIndPrimitive(T_DIALOGUE_TERMINATION_IND_PRIMITIVE & tind_term_primitive)
{
  _trnFSM.updateTransaction(tind_term_primitive);

  tind_term_primitive.setDialogueId(getDialogueId());

  TimeoutMonitor::getInstance().cancel(getDialogueTimeoutId());

  notifyTCUser(tind_term_primitive);

  TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(tind_term_primitive.getDialogueId());
}

template<class T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE>
void
TDialogueServiceData::handleDialogueNotTerminationIndPrimitive(T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE & tind_not_term_primitive)
{
  _trnFSM.updateTransaction(tind_not_term_primitive);

  tind_not_term_primitive.setDialogueId(getDialogueId());

  notifyTCUser(tind_not_term_primitive);
}

template<class T_DIALOGUE_REQUEST_PRIMITIVE>
void
TDialogueServiceData::handleDialogueRequestPrimitive(T_DIALOGUE_REQUEST_PRIMITIVE* tDlgReqPrimitive)
{
  _trnFSM.updateTransaction(*tDlgReqPrimitive);

  tDlgReqPrimitive->setTransactionId(_trnFSM.getTransactionId());

  smsc::core::synchronization::MutexGuard synchronize(_lock_forAppCtxUpdate);
  if ( !_isDialogueWasAcked ) {
    _isDialogueWasAcked = true;
    if ( tDlgReqPrimitive->getAppCtx() )
      _applicationContext = *tDlgReqPrimitive->getAppCtx();
  }
  // What is the strangeness?!
  tDlgReqPrimitive->setAppCtx(_applicationContext);
}

template<class T_DIALOGUE_IND_PRIMITIVE>
void
TDialogueServiceData::notifyTCUser(T_DIALOGUE_IND_PRIMITIVE & tc_ind_primitive)
{
  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forCallToTDlgHndlrIface);
    _tDlgHndlrIface->updateDialogue(tc_ind_primitive);
  } catch (const std::exception& ex) {
    formPAbortIndication(tc_ind_primitive.getDialogueId(), PAbort::p_resourceLimitation,
                         _tDlgHndlrIface);
    formPAbortRequest(tc_ind_primitive.getTransactionId(), PAbort::p_resourceLimitation);
  }
}

