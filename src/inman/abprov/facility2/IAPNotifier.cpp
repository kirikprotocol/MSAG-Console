#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/IAPNotifier.hpp"

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
  if (ini_threads > _iniThreads)
    _iniThreads = ini_threads;
  _taskPool.reserveTasks(_iniThreads);
  if (max_threads)
    setMaxThreads(max_threads);
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
  NotificationTask * pTask = _taskPool.allcTask();
  if (pTask) {
    if (pTask->init(*_qrsStore, ab_id, _logger)) {
      smsc_log_debug(_logger, "%s: processing %s(%s)", _logId,
                     pTask->taskName(), ab_id.getSignals());
      startTask(pTask);
      return true;
    }
    smsc_log_error(_logger, "%s: non-existent query for %s", _logId, ab_id.getSignals());
    _taskPool.rlseTask(pTask);
  } else {
    smsc_log_error(_logger, "%s: task pool is exhausted", _logId);
  }
  return false;
}

/* ************************************************************************** *
 * class IAPNotifier::NTFTaskPool implementation:
 * ************************************************************************** */
void IAPNotifier::NTFTaskPool::reserveTasks(uint16_t num_tasks)
{
  MutexGuard grd(_sync);
  _pool.reserve(num_tasks); 
}

//Returns NULL if no task available
IAPNotifier::NotificationTask * IAPNotifier::NTFTaskPool::allcTask(void)
{
  MutexGuard grd(_sync);
  TaskPool::PooledObj * rval = _pool.allcObj();
  if (rval)
    rval->setOwner(*this);
  return rval;
}

void IAPNotifier::NTFTaskPool::rlseTask(IAPNotifier::NotificationTask * p_task)
{
  MutexGuard grd(_sync);
  _pool.rlseObj(static_cast<TaskPool::PooledObj*>(p_task));
}


/* ************************************************************************** *
 * class IAPNotifier::NotificationTask implementation:
 * ************************************************************************** */
const TimeSlice IAPNotifier::NotificationTask::_dflt_wait_tmo(50, TimeSlice::tuMSecs); //50 msec

bool IAPNotifier::NotificationTask::init(
  IAPQueriesStore & qrs_store, const AbonentId & ab_id, Logger * use_log/* = NULL*/)
{
  isStopping = isReleased = false;
  _qGrd = qrs_store.getQuery(ab_id, false);
  if (!(_logger = use_log))
    _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);

  return (_qGrd.get() != NULL);
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
  _owner->rlseTask(this);
}

} //iaprvd
} //inman
} //smsc


