#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/asynconn/PckNotifier.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * class PckNotifier implementation:
 * ************************************************************************** */
const TimeSlice  PckNotifier::_dflt_ShutdownTmo(300, TimeSlice::tuMSecs);

void PckNotifier::init(const char * use_ident, Logger * use_log/* = NULL*/)
{
  snprintf(_logId, sizeof(_logId)-1, "%s.Ntfr", use_ident);
  if (!(_logger = use_log))
    _logger = Logger::getInstance(ASYNCONN_DFLT_LOGGER);
}

void PckNotifier::setThreads(uint16_t ini_threads, uint16_t max_threads/* = 0*/)
{
  setMaxThreads(max_threads);
  if (max_threads && (ini_threads > max_threads))
    ini_threads = max_threads;
  if (ini_threads > _iniThreads) {
    _iniThreads = ini_threads;
    _taskPool.reserveTasks(_iniThreads);
  }
}

void PckNotifier::stop(const TimeSlice * use_tmo/* = NULL*/)
{
  if (!use_tmo)
    use_tmo = &_dflt_ShutdownTmo;
  shutdown(use_tmo->Units(), use_tmo->Value());
}

// ------------------------------------------
// -- PckBufferRefereeIface interface methods
// ------------------------------------------
//Returns false if event cann't be processed by referee.
//Starts a threaded task that processes query event.
bool PckNotifier::onPacketEvent(const PckBufferGuard & use_pck)
{
  PckEventTask * pTask = _taskPool.allcTask();
  if (pTask) {
    if (pTask->init(use_pck, _lsrList, _logId, _logger) && startTask(pTask)) {
      smsc_log_debug(_logger, "%s: activated %s", _logId, pTask->taskName());
      return true;
    }
    smsc_log_error(_logger, "%s: failed to activate task", _logId);
    _taskPool.rlseTask(pTask);
  } else {
    smsc_log_error(_logger, "%s: task pool is exhausted: %u of %u", _logId,
                   (unsigned)_taskPool.usage(), (unsigned)_taskPool.capacity());
  }
  return false;
}

/* ************************************************************************** *
 * class PckNotifier::EVTTaskPool implementation:
 * ************************************************************************** */
void PckNotifier::EVTTaskPool::reserveTasks(uint16_t num_tasks)
{
  MutexGuard grd(_sync);
  _pool.reserve(num_tasks); 
}

//Returns NULL if no task available
PckNotifier::PckEventTask * PckNotifier::EVTTaskPool::allcTask(void)
{
  MutexGuard grd(_sync);
  TaskPool::PooledObj * rval = _pool.allcObj();
  if (rval)
    rval->setOwner(*this);
  return rval;
}

void PckNotifier::EVTTaskPool::rlseTask(PckNotifier::PckEventTask * p_task)
{
  MutexGuard grd(_sync);
  _pool.rlseObj(static_cast<TaskPool::PooledObj*>(p_task));
}

//
uint16_t PckNotifier::EVTTaskPool::capacity(void) const
{
  MutexGuard grd(_sync);
  return _pool.capacity(); 
}
//
uint16_t PckNotifier::EVTTaskPool::usage(void) const
{
  MutexGuard grd(_sync);
  return _pool.usage();
}


/* ************************************************************************** *
 * class PckNotifier::PckEventTask implementation:
 * ************************************************************************** */
const TimeSlice PckNotifier::PckEventTask::_dflt_wait_tmo(50, TimeSlice::tuMSecs); //50 msec


bool PckNotifier::PckEventTask::init(const PckBufferGuard & use_pck,
                                     ListenersList & lsr_list,
                                     const char * log_id, Logger * use_log)
  /*throw()*/
{
  isStopping = isReleased = false;
  _pckGrd = use_pck;
  _logger = use_log;
  _lsrList = &lsr_list;
  snprintf(_logId, sizeof(_logId)-1, "%s[%u]", log_id, (unsigned)_unqIdx);
  return (_pckGrd.get() != NULL);
}
// -------------------------------------------
// -- ThreadedTask interface methods
// -------------------------------------------

int PckNotifier::PckEventTask::Execute(void) /*throw()*/
{
  if (!_pckGrd.get())
    return -1;

  if (_lsrList->empty()) {
    if (_pckGrd->isComplete()) {
      smsc_log_warn(_logger, "%s: no listeners to process packet event(status %s, data %u bytes)",
                 _logId, PckAccumulatorIface::nmStatus(_pckGrd->_accStatus),
                 (unsigned)_pckGrd->pckBuf().getDataSize());
    }
  } else {
    bool pckDone = false;
    for (ListenersList::iterator it = _lsrList->begin(); !it.isEnd() && !pckDone; ++it) {
      if (isStopping) {
        Logger::LogLevel logLvl = _pckGrd->isComplete() ?  Logger::LEVEL_WARN : Logger::LEVEL_DEBUG;
        _logger->log(logLvl, "%s: packet event(status %s, data %u bytes) processing is cancelled",
                     _logId, PckAccumulatorIface::nmStatus(_pckGrd->_accStatus),
                     (unsigned)_pckGrd->pckBuf().getDataSize());
        break;
      }
      try {
        pckDone = _pckGrd->isComplete()
                    ? it->onPacketReceived(_pckGrd->_connId, _pckGrd->pckBuf())
                    : it->onConnectError(_pckGrd->_connId, _pckGrd->_accStatus, _pckGrd->_exc.get());
      } catch (const std::exception & lexc) {
        smsc_log_error(_logger, "%s: listener exception: %s", _logId, lexc.what());
      } catch (...) {
        smsc_log_error(_logger, "%s: listener exception: <unknown>", _logId);
      }
    }
  }
  return 0;
}

void PckNotifier::PckEventTask::onRelease(void)
{
  _pckGrd.release();
  isReleased = true;
  _owner->rlseTask(this);
}

} //interaction
} //inman
} //smsc


