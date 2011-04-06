/* ************************************************************************** *
 * IAProvider notifier: pool of threads handling IAPQuery FSM switching events.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_NOTIFIER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_NOTIFIER_HPP

#include "core/threads/ThreadPool.hpp"
#include "inman/common/IDXObjPool_T.hpp"
#include "inman/abprov/facility2/IAPQueriesStore.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

using smsc::core::synchronization::TimeSlice;

class IAPNotifier : protected smsc::core::threads::ThreadPool,
                    public IAPQueryRefereeIface {
public:
  static const unsigned   _MAX_IDENT_SZ = 32;
  static const TimeSlice  _dflt_ShutdownTmo; //300 millisecs

  IAPNotifier(const char * use_ident, IAPQueriesStore & qrs_store,
              Logger * use_log = NULL);
  virtual ~IAPNotifier()
  { }

  //Note: (max_threads == 0) means no limit
  void init(uint16_t ini_threads, uint16_t max_threads = 0);
  //
  void start(void)  { preCreateThreads(_iniThreads); }
  //
  using smsc::core::threads::ThreadPool::stopNotify;
  //
  using smsc::core::threads::ThreadPool::isRunning;
  //
  void stop(const TimeSlice * use_tmo = NULL);

  // ------------------------------------------
  // -- IAPQueryRefereeIface interface methods
  // ------------------------------------------
  //Returns false if event cann't be processed by referee.
  //Starts a threaded task that processes query event.
  virtual bool onQueryEvent(AbonentId ab_id);

protected:
  class NTFTaskPool; //forward declaration of NotificationTasks pool

  //Handles referred IAPQuery FSM switching events.
  class NotificationTask : public smsc::core::threads::ThreadedTask {
  private:
    using smsc::core::threads::ThreadedTask::setDelOnCompletion;

    NTFTaskPool *     _owner;
    IAPQueryGuard     _qGrd;
    Logger *          _logger;

  protected:
    static const TimeSlice _dflt_wait_tmo; //50 msec

    //NOTE: task is reusable
    explicit NotificationTask() : smsc::core::threads::ThreadedTask(false)
      , _owner(NULL), _logger(NULL)
    { }
    //
    virtual ~NotificationTask()
    { }

    friend class NTFTaskPool;
    //Note: this method called once right after task allocation
    void setOwner(NTFTaskPool & use_owner) { _owner = &use_owner; }

  public:
    //Returns false if there is no active query in storage associated with given abonent.
    bool init(IAPQueriesStore & qrs_store, const AbonentId & ab_id, Logger * use_log = NULL);

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char * taskName(void);
    virtual int   Execute(void);
    virtual void  onRelease(void);
  };

  class NTFTaskPool {
  protected:
    typedef smsc::util::UNQObjPool_T<NotificationTask, uint16_t> TaskPool;

    mutable Mutex     _sync;
    TaskPool          _pool; //NOTE: tasks are reusable

  public:
    NTFTaskPool() : _pool(false)
    { }
    ~NTFTaskPool()
    { }

    inline void reserveTasks(uint16_t num_tasks);
    //Returns NULL if no task available
    inline NotificationTask * allcTask(void);
    //
    inline void rlseTask(NotificationTask * p_task);
  };

  /* - */
  uint16_t          _iniThreads;  //1 - by default
  IAPQueriesStore * _qrsStore;
  NTFTaskPool       _taskPool;
  char              _logId[sizeof("Ntfr[%s]") + _MAX_IDENT_SZ + 1];
  Logger *          _logger;
};


} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_NOTIFIER_HPP */

