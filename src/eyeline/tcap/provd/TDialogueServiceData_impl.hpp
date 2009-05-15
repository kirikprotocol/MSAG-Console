
template<class T_DIALOGUE_TERMINATION_IND_PRIMITIVE>
void
TDialogueServiceData::handleDialogueTerminationIndPrimitive(T_DIALOGUE_TERMINATION_IND_PRIMITIVE* tDlgTermIndPrimitive)
{
  _trnFSM.updateTransaction(*tDlgTermIndPrimitive);

  tDlgTermIndPrimitive->setDialogueId(getDialogueId());

  TimeoutMonitor::getInstance().cancel(getDialogueTimeoutId());

  notifyTCUser(tDlgTermIndPrimitive);

  TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(tDlgTermIndPrimitive->getDialogueId());
}

template<class T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE>
void
TDialogueServiceData::handleDialogueNotTerminationIndPrimitive(T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE* tDlgNotTermIndPrimitive)
{
  _trnFSM.updateTransaction(*tDlgNotTermIndPrimitive);

  tDlgNotTermIndPrimitive->setDialogueId(getDialogueId());

  notifyTCUser(tDlgNotTermIndPrimitive);
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
TDialogueServiceData::notifyTCUser(T_DIALOGUE_IND_PRIMITIVE* tcIndPrimitive)
{
  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forCallToTDlgHndlrIface);
    _tDlgHndlrIface->updateDialogue(*tcIndPrimitive);
  } catch (std::exception& ex) {
    formPAbortIndication(tcIndPrimitive->getDialogueId(), TDialogueHandlingPrimitive::p_resourceLimitation,
                         _tDlgHndlrIface);
    formPAbortRequest(tcIndPrimitive->getTransactionId(), TDialogueHandlingPrimitive::p_resourceLimitation);
  }
}

