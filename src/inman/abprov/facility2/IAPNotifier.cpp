#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/IAPNotifier.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPNotifier implementation:
 * ************************************************************************** */
const TimeSlice  IAPNotifier::_dflt_ShutdownTmo(300, TimeSlice::tuMSecs);

IAPNotifier::IAPNotifier(const char * use_ident, Logger * use_log/* = NULL*/)
  : _iniThreads(1)
{
  snprintf(_logId, sizeof(_logId)-1, "Ntfr[%s]", use_ident);
  if (!(_logger = use_log))
    _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);
}

void IAPNotifier::init(uint16_t ini_threads, uint16_t max_threads/* = 0*/)
{
  setMaxThreads(max_threads);
  if (max_threads && (ini_threads > max_threads))
    ini_threads = max_threads;
  if (ini_threads > _iniThreads)
    _iniThreads = ini_threads;
  _taskPool.reserve(_iniThreads);
}

void IAPNotifier::stop(const TimeSlice * use_tmo/* = NULL*/)
{
  if (!use_tmo)
    use_tmo = &_dflt_ShutdownTmo;
  shutdown(use_tmo->Units(), use_tmo->Value());
}

// ------------------------------------------
// -- IAPQueryRefereeIface interface methods
// ------------------------------------------
//Returns false if event cann't be processed by referee.
//Starts a threaded task that processes query event.
bool IAPNotifier::onQueryEvent(const IAPQueryRef & p_qry)
{
  NTFTaskRef  pTask = _taskPool.allcObj();
  if (!pTask.empty()) {
    if (pTask->init(pTask, p_qry, _logger) && startTask(pTask.get())) {
      smsc_log_debug(_logger, "%s: processing %s(%s)", _logId,
                     pTask->taskName(), p_qry->getAbonentId().getSignals());
      return true;
    }
    smsc_log_error(_logger, "%s: non-existent query for %s", _logId, p_qry->getAbonentId().getSignals());
    pTask->onRelease();
  } else {
    smsc_log_error(_logger, "%s: task pool is exhausted: %u of %u", _logId,
                   (unsigned)_taskPool.usage(), (unsigned)_taskPool.capacity());
  }
  return false;
}

/* ************************************************************************** *
 * class IAPNotifier::NotificationTask implementation:
 * ************************************************************************** */
bool IAPNotifier::NotificationTask::init(const NTFTaskGuard & task_grd,
                                         const IAPQueryRef & p_qry, Logger * use_log/* = NULL*/)
{
  _qGrd = p_qry;
  if (_qGrd.get()) {
    isStopping = isReleased = false;
    if (!(_logger = use_log))
      _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);
    _thisGrd = task_grd;
    return true;
  }
  return false;
}
// -------------------------------------------
// -- ThreadedTask interface methods
// -------------------------------------------
const char * IAPNotifier::NotificationTask::taskName(void)
{
  return _qGrd.get() ? _qGrd->taskName() : "unknown";
}

int IAPNotifier::NotificationTask::Execute(void)
{
  if (!_qGrd.get())
    return -1;

  bool doReport = false;
  {
    MutexGuard  grd(*_qGrd.get());
    if (_qGrd->getStage() == IAPQueryAC::qryReporting) {
      if (!isStopping) {
        smsc_log_debug(_logger, "%s(%s): reporting ..",
                       _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
        doReport = (_qGrd->notifyListeners() == IAPQueryAC::procNeedReport);
      } else {
        smsc_log_debug(_logger, "%s(%s): cancelling ..",
                       _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
        //never returns 'procLater' here
        doReport = (_qGrd->cancel(true) == IAPQueryAC::procNeedReport);
      }
    }
    if (!doReport && (_qGrd->getStage() == IAPQueryAC::qryStopping)) {
      smsc_log_debug(_logger, "%s(%s): stopping ..", 
                     _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
      //never returns 'procLater' here
      doReport = (_qGrd->cancel(true) == IAPQueryAC::procNeedReport);
    }
    if (_qGrd->getStage() == IAPQueryAC::qryDone) {
      smsc_log_debug(_logger, "%s(%s): releasing ..", 
                     _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
    } else if (!doReport) {
      smsc_log_warn(_logger, "%s(%s): onQueryEvent() at stage(%s)",
                     _qGrd->taskName(), _qGrd->getAbonentId().getSignals(), _qGrd->nmStage());
    }
  }
  if (doReport)
    _qGrd->reportThis();
  return 0;
}

void IAPNotifier::NotificationTask::onRelease(void)
{
  _qGrd.release(); //release query ref(entire query if ref is a last one)
  isReleased = true;
  _thisGrd.release();
}

} //iaprvd
} //inman
} //smsc


