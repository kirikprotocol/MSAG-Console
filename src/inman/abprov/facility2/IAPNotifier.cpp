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

IAPNotifier::IAPNotifier(const char * use_ident, IAPQueriesStore & qrs_store,
                         Logger * use_log/* = NULL*/)
  : _iniThreads(1), _qrsStore(&qrs_store)
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
bool IAPNotifier::onQueryEvent(AbonentId ab_id)
{
  NTFTaskRef  pTask = _taskPool.allcObj();
  if (!pTask.empty()) {
    if (pTask->init(pTask, *_qrsStore, ab_id, _logger) && startTask(pTask.get())) {
      smsc_log_debug(_logger, "%s: processing %s(%s)", _logId,
                     pTask->taskName(), ab_id.getSignals());
      return true;
    }
    smsc_log_error(_logger, "%s: non-existent query for %s", _logId, ab_id.getSignals());
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
const TimeSlice IAPNotifier::NotificationTask::_dflt_wait_tmo(50, TimeSlice::tuMSecs); //50 msec

bool IAPNotifier::NotificationTask::init(const NTFTaskGuard & task_grd,
  IAPQueriesStore & qrs_store, const AbonentId & ab_id, Logger * use_log/* = NULL*/)
{
  _qGrd = qrs_store.getQuery(ab_id, false);
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
  {
    MutexGuard  grd(*_qGrd.get());
    if (!isStopping) {
      if (_qGrd->getStage() == IAPQueryAC::qryReporting) {
        smsc_log_debug(_logger, "%s(%s): reporting ",
                       _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
        _qGrd->notifyListeners();
        return 0;
      }
      while ((_qGrd->getStage() == IAPQueryAC::qryDone) && !_qGrd->isToRelease())
        _qGrd->wait(_dflt_wait_tmo);
    } //else: task was cancelled, handle only query object releasing

    if (_qGrd->getStage() == IAPQueryAC::qryDone) {
      smsc_log_debug(_logger, "%s(%s): releasing ..", 
                     _qGrd->taskName(), _qGrd->getAbonentId().getSignals());
      _qGrd.rlseQuery();
    } else {
      smsc_log_warn(_logger, "%s(%s): processEvent() at stage %u", 
                     _qGrd->taskName(), _qGrd->getAbonentId().getSignals(), (unsigned)_qGrd->getStage());
    }
  }
  return 0;
}

void IAPNotifier::NotificationTask::onRelease(void)
{
  _qGrd.clear(); //release query ref(entire query if ref is a last one)
  isReleased = true;
  _thisGrd.release();
}

} //iaprvd
} //inman
} //smsc


