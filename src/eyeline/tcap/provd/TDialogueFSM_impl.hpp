
//NOTE: TDialogueFSM and TCUser must be locked upon entry!!!
template<class TDLG_IND_COMPOSER>
  bool TDialogueFSM::notifyLocalTCUser(TDlgIndHandlerGuard & grd_user,
                                       TDLG_IND_COMPOSER & tr_ind) /* throw() */
{
  smsc::core::synchronization::ReverseMutexGuard rGrd(_sync); //TODO: analyze necessarity!

  bool rval = true;
  try {
    if (grd_user.get())
      grd_user->updateDialogue(tr_ind.TInd());
  } catch (const std::exception & ex) {
    smsc_log_error(_logger, "%s: TC User exception on %s: %s",
                    _logId, tr_ind.getIdent(), ex.what());
    rval = false;
  } catch (...) {
    smsc_log_error(_logger, "%s: TC User exception on %s",
                    _logId, tr_ind.getIdent());
    rval = false;
  }
  return rval;
}

