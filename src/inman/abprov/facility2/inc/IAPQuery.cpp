#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/inc/IAPQuery.hpp"

using smsc::util::URCRegistry;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPQueryAC implementation:
 * ************************************************************************** */
const char * IAPQueryAC::nmStage(ProcStage_e use_val)
{
  switch (use_val) {
  case qryIdle:       return "qryIdle";
  case qryStarted:    return "qryStarted";
  case qryResulted:   return "qryResulted";
  case qryReporting:  return "qryReporting";
  case qryStopping:   return "qryStopping";
  case qryDone:       return "qryDone";
  default:;
  }
  return "unknown";
}

void IAPQueryAC::init(IAPQueryRefereeIface & use_owner, const AbonentId & ab_id)
{
  _owner = &use_owner;
  _abId = ab_id;
  _stages.clear();
  _lsrList.clear();
  _qStatus = IAPQStatus::iqOk;
  _qError = 0;
  _abInfo.clear();
  _exc.clear();
  ++_usage;
  mkTaskName();
  if (!_logger)
    _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);
}

//Cancels query execution, switches FSM to qryStopping/qryDone state.
//If argument 'do_wait' is set, blocks until qryStopping/qryDone state
//is reached. Note: Listeners aren't notified.
//Returns false if listener is already targeted and query waits for its mutex.
IAPQueryAC::ProcResult_e
  IAPQueryAC::cancel(bool do_wait) /*throw()*/
{
  while (hasListenerAimed()) { //qryReporting stage
    if (!do_wait)
      return IAPQueryAC::procLater;
    this->wait();
  }

  if (getStage() == qryDone)
    return IAPQueryAC::procOk;

  if (getStage() == qryStopping) {
    if (do_wait) {
      finalize(); //blocks until all resources are released
      setStageNotify(qryDone);
    }
    return IAPQueryAC::procOk;
  }
  //here getStage() < qryStopping
  _lsrList.clear();
  _qStatus = IAPQStatus::iqCancelled;
  setStageNotify(qryStopping);
  return IAPQueryAC::procNeedReport;
}

void IAPQueryAC::mkTaskName(void)
{
  char buf[_idTaskSZ + 1];
  snprintf(buf, sizeof(buf)-1, "[%u:%lu]", this->getUIdx(), _usage);
  _tName = taskType().c_str();
  _tName += buf;
}

//Returns:
// 'procOk'    - if listener is registered and will be notified as query yelds result.
// 'procLater' - if query is stopping/already completed.
IAPQueryAC::ProcResult_e
  IAPQueryAC::addListener(IAPQueryListenerITF & pf_cb)
{
  IAPQueryAC::ProcResult_e res = IAPQueryAC::procOk;

  if (_stages._current <= qryReporting)
    _lsrList.push_back(ListenerInfo(&pf_cb));
  else
    res = IAPQueryAC::procLater;

  this->notify();
  return res;
}

//Returns:
// 'procOk'    - if succeeded.
// 'procLater' - if listener is already targeted and query waits for its mutex.
IAPQueryAC::ProcResult_e
  IAPQueryAC::removeListener(IAPQueryListenerITF & pf_cb)
{
  QueryListeners::iterator it = _lsrList.begin();
  for (; it != _lsrList.end(); ++it) {
    if (it->_ptr == &pf_cb) {
      if (it->_isAimed)
        return IAPQueryAC::procLater;
      _lsrList.erase(it);
      return IAPQueryAC::procOk;
    }
  }
  return IAPQueryAC::procOk;
}

//Is called by query referee.
IAPQueryAC::ProcResult_e
  IAPQueryAC::notifyListeners(void) /*throw()*/
{
  if (getStage() > qryReporting)
    return IAPQueryAC::procOk;

  while (!_lsrList.empty() && (getStage() == qryReporting)) {
    QueryListeners::iterator it = _lsrList.begin();
    bool res = it->_isAimed = true;
    {
      ReverseMutexGuard rGrd(*this);
      try { res = it->_ptr->onIAPQueried(_abId, _abInfo, _qError);
      } catch (const std::exception & exc) {
        smsc_log_error(_logger, "%s(%s): listener exception: %s", 
                       taskName(), _abId.getSignals(), exc.what());
      } catch (...) {
        smsc_log_error(_logger, "%s(%s): listener exception: 'unknown'", 
                       taskName(), _abId.getSignals());
      }
    }
    if (res)
      _lsrList.pop_front();
    else { //rereport is requested, move listener to end of queue
      it->_isAimed = false;
      if (++it != _lsrList.end())
        _lsrList.splice(_lsrList.end(), _lsrList, _lsrList.begin());
    }
  }
  if (getStage() > qryReporting)
    return IAPQueryAC::procOk;

  setStageNotify(qryStopping);
  return procNeedReport;
}

std::string IAPQueryAC::status2Str(void) const
{
  std::string st;

  switch (_qStatus) {
  case IAPQStatus::iqOk:  st += "finished";
    break;
  case IAPQStatus::iqCancelled: st += "cancelled";
    break;
  case IAPQStatus::iqTimeout: {
    st += "timed out";
    if (!_exc.empty()) {
      st += ", "; st += _exc;
    }
  } break;
  default: {
    st += "failed";
    if (!_exc.empty()) {
      st += ", "; st += _exc;
    } else if (_qError) {
      st += ", "; st += URCRegistry::explainHash(_qError);
    }
  }} /* eosw */
  return st;
}

} //iaprvd
} //inman
} //smsc


