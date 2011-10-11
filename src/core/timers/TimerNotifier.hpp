/* ************************************************************************** *
 * MT Realtime timers monitoring facility. 
 * TimerNotifier: thread pool serving timers signalling.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMER_NOTIFIER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMER_NOTIFIER_HPP

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/timers/TimerFSM.hpp"

#define TIMERS_MONITOR_DFLT_LOGGER "smsc.core.timers"

namespace smsc {
namespace core {
namespace timers {

using smsc::logger::Logger;

class TimerNotifier : protected smsc::core::threads::ThreadPool {
public:
  static const size_t     k_maxIdentPfxLen = 16;
  static const size_t     k_maxIdStrLen = sizeof("TmNtfr[%s]") + k_maxIdentPfxLen;
  static const TimeSlice  k_dfltShutdownTmo; //units: millisecs

  typedef smsc::core::buffers::FixedLengthString<k_maxIdStrLen + 1>  IdentStr_t;

  explicit TimerNotifier(const char * use_ident, Logger * use_log = NULL);
  ~TimerNotifier();

  //Note: (max_threads == 0) means no limit
  void init(uint16_t ini_threads, uint16_t max_threads = 0);
  //
  void start(void)  { preCreateThreads(mIniThreads); }
  //
  using smsc::core::threads::ThreadPool::stopNotify;
  //
  using smsc::core::threads::ThreadPool::isRunning;
  //
  void stop(const TimeSlice * use_tmo = NULL);

  //Starts a threaded task that processes timer signalling.
  //Returns false if event cann't be processed by notifier.
  bool onTimerEvent(const TimerRef & p_tmr);

protected:
  typedef smsc::core::buffers::IDAPoolCoreAC_T<uint16_t>::ObjGuard  NTFTaskGuard;

  //Handles referred IAPQuery FSM switching events.
  class NotificationTask : public smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> {
  public:
    //Returns false if there is no active query in storage associated with given abonent.
    bool init(const NTFTaskGuard & task_grd, const TimerRef & p_tmr,
              const char * log_id, Logger * use_log = NULL);

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char * taskName(void) { return mLogId; }
    virtual int   Execute(void);
    virtual void  onRelease(void);

  private:
    typedef smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> Base_T;
    using Base_T::setDelOnCompletion;

    NTFTaskGuard  mThisGrd; //guards this task until it complete Excecute(); 
    TimerRef      mTmrRef;
    Logger *      mLogger;
    char          mLogId[TimerNotifier::k_maxIdStrLen + sizeof("[%u]") + sizeof(uint16_t)*3];

  protected:
    // -----------------------------------------------
    // -- UniqueObj_T<, uint16_t> interface methods
    // -----------------------------------------------
    explicit NotificationTask(uint16_t use_idx) : Base_T(use_idx), mLogger(0)
    {
      mLogId[0] = 0;
      this->setDelOnCompletion(false); //task are reusable !!!
    }
    //
    virtual ~NotificationTask()
    { }
  };
  //NOTE: tasks are reusable
  typedef smsc::core::buffers::IntrusivePoolOfUnique_T<NotificationTask, uint16_t> NTFTaskPool;
  typedef NTFTaskPool::ObjRef NTFTaskRef;

private:
  /* - */
  uint16_t          mIniThreads;  //1 - by default
  NTFTaskPool       mTaskPool;
  char              mLogId[k_maxIdStrLen + 1];
  Logger *          mLogger;
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMER_NOTIFIER_HPP */

