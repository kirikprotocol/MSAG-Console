/* ************************************************************************** *
 * PckNotifierr: pool of threads handling PckBuffer accumulation events.
 * ************************************************************************** */
#ifndef SMSC_INMAN_ASYNCONN_PCK_NOTIFIER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_ASYNCONN_PCK_NOTIFIER

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"

#include "inman/common/RFCPList_T.hpp"
#include "inman/common/IDXObjPool_T.hpp"

#include "inman/interaction/PckListenerDefs.hpp"
#include "inman/interaction/asynconn/PckBufferStore.hpp"

#define ASYNCONN_DFLT_LOGGER "smsc.inman.asconn"

namespace smsc {
namespace inman {
namespace interaction {

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::TimeSlice;

class PckNotifier : protected smsc::core::threads::ThreadPool,
                    public PckBufferRefereeIface {
public:
  static const unsigned   _MAX_IDENT_SZ = 32;
  static const TimeSlice  _dflt_ShutdownTmo; //300 millisecs

  PckNotifier() : _iniThreads(1), _logger(NULL)
  {
    _logId[0] = 0;
  }
  virtual ~PckNotifier()
  {
    if (isRunning())
      stop();
  }

  void init(const char * use_ident, Logger * use_log = NULL);

  //
  using smsc::core::threads::ThreadPool::getMaxThreads;
  //Note: (max_threads == 0) means no limit
  //      (max_threads == 1) means consecuitive events processing
  void setThreads(uint16_t ini_threads, uint16_t max_threads = 0);
  //
  bool start(void)  { return (preCreateThreads(_iniThreads) > 0); }
  //
  using smsc::core::threads::ThreadPool::stopNotify;
  //
  using smsc::core::threads::ThreadPool::isRunning;
  //
  void stop(const TimeSlice * use_tmo = NULL);

  //
  void addListener(PacketListenerIface & use_lstr)
  {
    _lsrList.push_back(&use_lstr);
  }
  //
  void removeListener(PacketListenerIface & use_lstr)
  {
    _lsrList.remove(&use_lstr);
  }
  //
  void clearListeners(void)
  {
    _lsrList.clear();
  }

  // ------------------------------------------
  // -- PckBufferRefereeIface interface methods
  // ------------------------------------------
  //Returns false if event cann't be processed by referee.
  //Starts a threaded task that processes query event.
  virtual bool onPacketEvent(const PckBufferGuard & use_pck);

protected:
  //
  typedef smsc::util::RFCPList_T<PacketListenerIface> ListenersList;

  class EVTTaskPool; //forward declaration of PckEventTasks pool

  //Process packet accumulation event.
  class PckEventTask : public smsc::core::threads::ThreadedTask {
  private:
    using smsc::core::threads::ThreadedTask::setDelOnCompletion;

    const uint16_t    _unqIdx; //unique index of this object
    EVTTaskPool *     _owner;
    PckBufferGuard    _pckGrd;
    char              _logId[sizeof("%s.Ntfr[%u]") + _MAX_IDENT_SZ + sizeof(uint16_t)*3];
    Logger *          _logger;
    ListenersList *   _lsrList;

  protected:
    static const TimeSlice _dflt_wait_tmo; //50 msec

    // -----------------------------------------------
    // -- IndexedObj_T<, uint16_t> interface methods
    // -----------------------------------------------
    explicit PckEventTask(uint16_t use_idx)
      : smsc::core::threads::ThreadedTask(false), _unqIdx(use_idx)
      , _owner(NULL), _logger(NULL)
    {
      _logId[0] = 0;
    }
    //
    virtual ~PckEventTask()
    { }

    friend class EVTTaskPool;
    //Note: this method called once right after task allocation
    void setOwner(EVTTaskPool & use_owner) { _owner = &use_owner; }

  public:
    // -----------------------------------------------
    // -- IndexedObj_T<, uint16_t> interface methods
    // -----------------------------------------------
    uint16_t getIndex(void) const { return _unqIdx; }

    //Returns false if PacketBuffer is no longer available
    bool init(const PckBufferGuard & use_pck,
              ListenersList & lsr_list,
              const char * log_id, Logger * use_log) /*throw()*/;

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char * taskName(void) { return _logId; }
    virtual int   Execute(void) /*throw()*/;
    virtual void  onRelease(void);
  };

  class EVTTaskPool {
  protected:
    typedef smsc::util::IDXObjPool_T<PckEventTask, uint16_t> TaskPool;

    mutable Mutex     _sync;
    TaskPool          _pool; //NOTE: tasks are reusable

  public:
    EVTTaskPool() : _pool(false)
    { }
    ~EVTTaskPool()
    { }

    void reserveTasks(uint16_t num_tasks);
    //Returns NULL if no task available
    PckEventTask * allcTask(void);
    //
    void rlseTask(PckEventTask * p_task);
    //
    uint16_t capacity(void) const;
    //
    uint16_t usage(void) const;
  };

  /* - */
  uint16_t            _iniThreads;  //1 - by default
  EVTTaskPool         _taskPool;
  char                _logId[sizeof("%s.Ntfr") + _MAX_IDENT_SZ];
  Logger *            _logger;
  ListenersList       _lsrList;
};


} //interaction
} //inman
} //smsc

#endif /* SMSC_INMAN_ASYNCONN_PCK_NOTIFIER */

