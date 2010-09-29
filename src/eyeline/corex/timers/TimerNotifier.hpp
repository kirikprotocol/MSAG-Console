/* ************************************************************************** *
 * MT Realtime timers watching facility: thread pool serving timers signalling.
 * ************************************************************************** */
#ifndef _CORE_TIMERS_TIMER_NOTIFIER_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_TIMER_NOTIFIER_HPP

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "eyeline/corex/timers/StopWatchStore.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::logger::Logger;

class TimerNotifier : protected smsc::core::threads::ThreadPool {
protected:
  static const size_t _MAX_IDENT_STR_LEN = sizeof("TmNtfr[]") + TimeWatcherIface::_MAX_IDENT_LEN;
  typedef smsc::core::buffers::FixedLengthString<_MAX_IDENT_STR_LEN>  IdentStr_t;

  class SWSignalingTask;

  class SWSTaskStore {
  protected:
    typedef std::vector<SWSignalingTask *> TasksArray;
    typedef std::list<SWSignalingTask *> TasksPool;

    mutable Mutex     _sync;
    StopWatchStore &  _swStore;
    Logger *          _logger;
    TasksArray        _store;
    TasksPool         _pool;

  public:
    SWSTaskStore(StopWatchStore & sw_store, Logger * use_log = NULL)
      : _swStore(sw_store), _logger(use_log)
    {
      if (!_logger)
        _logger = Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
    }
    ~SWSTaskStore()
    {
      rlseAlltasks();
    }

    void reserveTasks(unsigned num_tasks);

    SWSignalingTask * allcTask(uint32_t sw_id);

    void rlseTask(SWSignalingTask * p_task);

    void rlseAlltasks(void);
  };


  //NOTE: task is reusable
  class SWSignalingTask : public smsc::core::threads::ThreadedTask {
  private:
    SWSTaskStore *    _owner;
    uint32_t          _swId;
    StopWatchStore *  _swStore;
    TimerIdent        _idStr;
    Logger *          _logger;

  protected:
    using smsc::core::threads::ThreadedTask::setDelOnCompletion;

  public:
    SWSignalingTask(SWSTaskStore & use_owner, Logger * use_log = NULL)
      : smsc::core::threads::ThreadedTask(false)
      , _owner(&use_owner), _swId(0), _swStore(0), _logger(use_log)
    {
      if (!_logger)
        _logger = Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
    }
    ~SWSignalingTask()
    { }

    void Init(StopWatchStore & sw_store, uint32_t sw_id)
    {
      _swId = sw_id; _swStore = &sw_store;
    }

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char* taskName(void);
    virtual int Execute(void);
    virtual void onRelease(void);
  };


  IdentStr_t    _ident;
  unsigned      _iniThreads;  //1 - by default
  SWSTaskStore  _taskStore;

public:
  static const TimeSlice  _dflt_Shutdown_Tmo;

  TimerNotifier(const TimeWatcherIdent & tmw_id, StopWatchStore & sw_store,
                Logger * use_log = NULL)
    : _ident("TmNtfr["), _iniThreads(1), _taskStore(sw_store, use_log)
  {
    _ident += tmw_id; _ident += ']';
  }
  ~TimerNotifier()
  {
    Stop(_dflt_Shutdown_Tmo.Units(), _dflt_Shutdown_Tmo.Value());
  }

  void Init(unsigned ini_threads, unsigned max_threads = 0)
  {
    if (ini_threads > _iniThreads)
      _taskStore.reserveTasks(_iniThreads = ini_threads);
    if (max_threads)
      setMaxThreads(max_threads);
  }

  const char * idStr(void) const { return _ident.c_str(); }

  void signalTimer(uint32_t sw_id)
  {
    startTask(_taskStore.allcTask(sw_id));
  }
  //
  bool isRunning(void) const
  {
    return getActiveThreads() != 0;
  }
  //
  bool Start(void)
  {
    _taskStore.reserveTasks(_iniThreads);
    preCreateThreads(_iniThreads);
    return true;
  }

  void StopNotify(void)
  {
    stopNotify();
  }

  void Stop(TimeSlice::UnitType_e time_unit, long use_tmo)
  {
    shutdown(time_unit, use_tmo);
  }
};


} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_TIMER_NOTIFIER_HPP */

