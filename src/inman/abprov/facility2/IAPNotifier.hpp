/* ************************************************************************** *
 * IAProvider notifier: pool of threads handling IAPQuery FSM switching events.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_NOTIFIER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_NOTIFIER_HPP

#include "core/threads/ThreadPool.hpp"
#include "core/buffers/IntrusivePoolOfUniqueT.hpp"
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
  typedef smsc::core::buffers::IDAPoolCoreAC_T<uint16_t>::ObjGuard  NTFTaskGuard;

  //Handles referred IAPQuery FSM switching events.
  class NotificationTask : public smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> {
  public:
    //Returns false if there is no active query in storage associated with given abonent.
    bool init(const NTFTaskGuard & task_grd, IAPQueriesStore & qrs_store,
              const AbonentId & ab_id, Logger * use_log = NULL);

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char * taskName(void);
    virtual int   Execute(void);
    virtual void  onRelease(void);

  private:
    typedef smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> Base_T;
    using Base_T::setDelOnCompletion;

    NTFTaskGuard      _thisGrd; //guards this task until it complete Excecute(); 
    IAPQueryGuard     _qGrd;
    Logger *          _logger;

  protected:
    static const TimeSlice _dflt_wait_tmo; //50 msec

    // -----------------------------------------------
    // -- UniqueObj_T<, uint16_t> interface methods
    // -----------------------------------------------
    explicit NotificationTask(uint16_t use_idx) : Base_T(use_idx), _logger(0)
    {
      this->setDelOnCompletion(false); //task are reusable !!!
    }
    //
    virtual ~NotificationTask()
    { }
  };
  //NOTE: tasks are reusable
  typedef smsc::core::buffers::IntrusivePoolOfUnique_T<NotificationTask, uint16_t> NTFTaskPool;
  typedef NTFTaskPool::ObjRef NTFTaskRef;

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

