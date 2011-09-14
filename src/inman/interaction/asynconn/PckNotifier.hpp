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
#include "core/buffers/IntrusivePoolOfUniqueT.hpp"

#include "inman/common/RFCPList_T.hpp"
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
  //  
  typedef smsc::core::buffers::IDAPoolCoreAC_T<uint16_t>::ObjGuard  EVTTaskGuard;

  //Processes packet accumulation event: notifies it to listeners.
  class PckEventTask : public smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> {
  public:
    //Returns false if PacketBuffer is no longer available
    bool init(const EVTTaskGuard & task_grd, const PckBufferGuard & use_pck,
              const ListenersList & lsr_list, const char * log_id, Logger * use_log) /*throw()*/;

    // -------------------------------------------
    // -- ThreadedTask interface methods
    // -------------------------------------------
    virtual const char * taskName(void) { return _logId; }
    virtual int   Execute(void) /*throw()*/;
    virtual void  onRelease(void);

  private:
    typedef smsc::core::buffers::UniqueObj_T<smsc::core::threads::ThreadedTask, uint16_t> Base_T;
    using Base_T::setDelOnCompletion;

    EVTTaskGuard        _thisGrd; //guards this task until it complete Excecute(); 
    PckBufferGuard        _pckGrd;
    char                  _logId[sizeof("%s.Ntfr[%u]") + _MAX_IDENT_SZ + sizeof(uint16_t)*3];
    Logger *              _logger;
    const ListenersList * _lsrList;

  protected:
    static const TimeSlice _dflt_wait_tmo; //50 msec

    // -----------------------------------------------
    // -- UniqueObj_T<, uint16_t> interface methods
    // -----------------------------------------------
    explicit PckEventTask(uint16_t use_idx) : Base_T(use_idx)
      , _logger(0), _lsrList(0)
    {
      _logId[0] = 0;
      this->setDelOnCompletion(false); //task are reusable !!!
    }
    //
    virtual ~PckEventTask()
    { }
  };

  //NOTE: tasks are reusable
  typedef smsc::core::buffers::IntrusivePoolOfUnique_T<PckEventTask, uint16_t> EVTTaskPool;
  typedef EVTTaskPool::ObjRef EVTTaskRef;

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

