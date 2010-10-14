#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TCLclIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void TCIndLclCancel::process(void) { _ownTask->process(); }
//void TCIndLclReject::process(void) { _ownTask->process(); }
void TIndLclPAbort::process(void) { _ownTask->process(); }
void TIndLclNotice::process(void) { _ownTask->process(); }


/* ********************************************************************** *
 * class TCLocalIndTask implementation:
 * ********************************************************************** */

const char * TCLocalIndTask::nmKind(IndKind_e ind_kind)
{
  switch (ind_kind) {
  case indTNotice: return "TNotice_IND";
  case indPAbort:  return "TPAbort_IND";
//  case indLReject: return "TCLReject_IND";
  case indLCancel: return "TCLCancel_IND";
  default:;
  }
  return "<undefined>_IND";
}

TDialogueId TCLocalIndTask::getDlgId(void) const
{
  if (_pInd._any) {
    switch (_kind) {
    case indLCancel: return _pInd._lCancel->getDialogueId();
//    case indLReject: return _pInd._lReject->getDialogueId();
    case indPAbort:  return _pInd._tPAbort->getDialogueId();
    case indTNotice:  return _pInd._tNotice->getDialogueId();
    default:;
    }
  }
  return 0;
}

void TCLocalIndTask::clear(void)
{
  if (_pInd._any) {
    switch (_kind) {
    case indLCancel: _pInd._lCancel->~TCIndLclCancel(); break;
//    case indLReject: _pInd._lReject->~TCIndLclReject(); break;
    case indPAbort:  _pInd._tPAbort->~TIndLclPAbort(); break;
    case indTNotice:  _pInd._tNotice->~TIndLclNotice(); break;
    default:;
    }
    _pInd._any = 0;
    _kind = indNone;
  }
  _logId.clear();
}

TCIndLclCancel & TCLocalIndTask::initLCancel(void)
{
  clear();
  _pInd._lCancel = new (_memObj._buf) TCIndLclCancel(*this);
  _kind = indLCancel;
  return *_pInd._lCancel;
}
//
/*
TCIndLclReject & TCLocalIndTask::initLReject(void)
{
  clear();
  _pInd._lReject = new (_memObj._buf) TCIndLclReject(*this);
  _kind = indLReject;
  return *_pInd._lReject;
}
*/
//
TIndLclPAbort & TCLocalIndTask::initPAbort(void)
{
  clear();
  _pInd._tPAbort = new (_memObj._buf) TIndLclPAbort(*this);
  _kind = indPAbort;
  return *_pInd._tPAbort;
}
//
TIndLclNotice & TCLocalIndTask::initNotice(void)
{
  clear();
  _pInd._tNotice = new (_memObj._buf) TIndLclNotice(*this);
  _kind = indTNotice;
  return *_pInd._tNotice;
}

// -------------------------------------------
// -- ThreadedTask interface methods
// -------------------------------------------
int TCLocalIndTask::Execute(void)
{
  if (!_pInd._any) {
    smsc_log_error(_logger, "TCLocalIndTask::Execute(): task isn't initialized");
    return -1;
  }

  try {
    switch (_kind) {
    case indPAbort: {
      TDlgRefPtr_t dlgFsm = 
          _dlgReg->getTDlgFSM(_pInd._tPAbort->getDialogueId());
      dlgFsm->updateDlgByIndication(*_pInd._tPAbort);
    } break;

    case indTNotice: {
      TDlgRefPtr_t dlgFsm = 
          _dlgReg->getTDlgFSM(_pInd._tNotice->getDialogueId());
      dlgFsm->updateDlgByIndication(*_pInd._tNotice);
    } break;

    case indLCancel: {
      TDlgRefPtr_t dlgFsm = 
          _dlgReg->getTDlgFSM(_pInd._lCancel->getDialogueId());
      dlgFsm->updateDlgByIndication(*_pInd._lCancel);
    } break;
/*
    case indLReject: {
      TDlgRefPtr_t dlgFsm = 
          _dlgReg->getTDlgFSM(_pInd._lReject->getDialogueId());
      dlgFsm->updateDlgByIndication(*_pInd._lReject);
    } break;
*/
    default:;
    }
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s: caught exception: \'%s\'", taskName(), exc.what());
  } catch (...) {
    smsc_log_error(_logger, "%s: caught unknown exception", taskName());
  }
  return 0;
}

} //provd
} //tcap
} //eyeline

