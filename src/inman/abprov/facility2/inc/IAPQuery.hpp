/* ************************************************************************** *
 * IAPQueryAC: Finite State Machine for INMan Abonent Provider queries.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_QUERY_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_QUERY_DEFS_HPP

#include <list>

#include "logger/Logger.h"

#include "util/UniqueObjT.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/IntrusivePoolOfIfaceT.hpp"

#include "inman/abprov/IAPErrors.hpp"
#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

using smsc::logger::Logger;

typedef uint16_t IAPQueryId;

class IAPQueryRefereeIface {
protected:
  virtual ~IAPQueryRefereeIface()
  { }

public:
  // -------------------------------------------------------
  // -- IAPQueryRefereeIface interface methods
  // -------------------------------------------------------
  virtual void onQueryEvent(IAPQueryId qry_id) /*throw()*/= 0;
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueryAC : public smsc::util::UniqueObj_T<smsc::core::synchronization::EventMonitor, IAPQueryId> {
public:
  //query FSM processing stage
  enum ProcStage_e {
    qryIdle = 0x00      //query is not initialized
  , qryStarted = 0x01   //query is initialized and started
  , qryResulted = 0x02  //query received a result
  , qryReporting = 0x04 //query is reporting its result to listeners
  , qryStopping = 0x08  //query is releasing active resources (connections, etc)
  , qryDone = 0x10      //query finished its processing cycle
                        //(NOTE: if query was cancelled result may not be valid)
  };
  //query action result
  enum ProcResult_e {
    procLater = 0x00      //action cann't be performed right now
  , procOk = 0x01         //action is succesfully completed
  , procNeedReport = 0x02 //action is succesfully completed, FSM is switched,
                          //so query must be reported to referee.
  };

  static const unsigned _nmTypeSZ = 32;
  static const unsigned _idTaskSZ = (unsigned)(sizeof("[%u:%lu]")
                                               + sizeof(IAPQueryId /*_qId*/)*3
                                               + sizeof(unsigned long/*usage*/)*3);
  static const unsigned _nmQuerySZ = _idTaskSZ + _nmTypeSZ;

  typedef core::buffers::FixedLengthString<_nmTypeSZ + 1>  TypeString_t;
  typedef core::buffers::FixedLengthString<_nmQuerySZ + 1>  QueryName_t;

  static const char * nmStage(ProcStage_e use_val);

  //Reports query state to its referee.
  //NOTE: MUST NOT be called on locked query!!!
  void reportThis(void) { _owner->onQueryEvent(this->getUIdx()); }

  // ------------------------------------------------------------
  // All following methods should be called on locked query!!!
  // ------------------------------------------------------------

  const char * nmStage(void) const { return nmStage(getStage()); }

  //Configures query: assigns abonentId, switches FSM to 'qryIdle' state.
  void init(IAPQueryRefereeIface & use_owner, const AbonentId & ab_number);
  //Cancels query execution (listeners aren't notified), switches FSM to
  //'qryStopping' state.
  //If argument 'do_wait' is set, blocks until qryDone state is reached.
  //Returns 'procLater' if listener is already targeted and query waits for its mutex.
  //NOTE: Switches FSM (Is called by query referee).
  ProcResult_e cancel(bool do_wait) /*throw()*/;
  //Appends given listener to query's listeners list.
  //Returns:
  // 'procOk'    - if listener is registered and will be notified as query yelds result.
  // 'procLater' - if query is stopping/already completed.
  //               In that case listener should be added to next query.
  ProcResult_e addListener(IAPQueryListenerITF & pf_cb);
  //Removes given listener from query's listeners list.
  //Returns:
  // 'procOk'    - if succeeded.
  // 'procLater' - if listener is already targeted and query waits for its mutex.
  ProcResult_e removeListener(IAPQueryListenerITF & pf_cb);
  //Reports query to all enqueued listeners, upon completion switches FSM to
  //'qryStopping' state.
  ProcResult_e notifyListeners(void) /*throw()*/;

  ProcStage_e getStage(void) const { return _stages._current; }

  bool hasListener(void) const { return !_lsrList.empty(); }
  bool hasListenerAimed(void) const
  {
    return (!_lsrList.empty() && _lsrList.front()._isAimed);
  }
  //Returns true if query is completed its processing cycle.
  bool isCompleted(void) const
  {
    return (getStage() == qryDone) && _lsrList.empty(); 
  }

  const char *                taskName(void)       const { return _tName.c_str(); }
  const AbonentId &           getAbonentId(void)   const { return _abId; }
  IAPQStatus::Code            getStatus(void)      const { return _qStatus; }
  std::string                 status2Str(void)     const;

  // -------------------------------------------------------
  // -- IAPQueryAC interface methods
  // -------------------------------------------------------
  virtual const TypeString_t & taskType(void) const /*throw()*/ = 0;
  //Starts query execution. May be called only at qryIdle state. 
  //In case of success switches FSM to qryStarted state,
  //otherwise to qryStopping or qryDone state.
  virtual ProcResult_e  start(void) /*throw()*/ = 0;

protected:
  //Blocks until all used resources are released.
  //Called only at qryStoppping state.
  virtual ProcResult_e  finalize(void) /*throw()*/ = 0;

  explicit IAPQueryAC(IAPQueryId q_id)
    : smsc::util::UniqueObj_T<smsc::core::synchronization::EventMonitor, IAPQueryId>(q_id)
    , _usage(0), _owner(NULL), _qStatus(IAPQStatus::iqOk), _qError(0), _logger(NULL)
  { }
  //
  virtual ~IAPQueryAC()
  { }

private:
  struct Stages {
    ProcStage_e _current;
    uint16_t    _mask;

    Stages() : _current(qryIdle), _mask(0)
    { }

    ProcStage_e get(void)               const { return _current; }
    bool        has(ProcStage_e fsm_st) const { return _mask & fsm_st; }

    void clear(void)              {_mask = 0; _current = qryIdle; }
    void set(ProcStage_e fsm_st)  { _mask |= (_current = fsm_st); }
  };

  struct ListenerInfo {
    bool                  _isAimed;
    IAPQueryListenerITF * _ptr;
    
    explicit ListenerInfo(IAPQueryListenerITF * use_ptr = NULL)
      : _isAimed(false), _ptr(use_ptr)
    { }
  };
  typedef std::list<ListenerInfo> QueryListeners;

  /* -- */
  unsigned long           _usage;  //counter of runs of this query object

protected:
  Stages                  _stages;
  QueryListeners          _lsrList;
  IAPQueryRefereeIface *  _owner;
  AbonentId               _abId;
  AbonentSubscription     _abInfo;
  QueryName_t             _tName;  //task name for logging
  IAPQStatus::Code        _qStatus;   //query completion status, is to return by Execute()
  RCHash                  _qError;
  std::string             _exc;   //query error/exception message
  Logger *                _logger;

  //Composes taskName, it's recommended to call it in successors constructor
  void mkTaskName(void);
  //
  void setStage(ProcStage_e fsm_st)
  {
    _stages.set(fsm_st);
  }
  //
  void setStageNotify(ProcStage_e fsm_st)
  {
    _stages.set(fsm_st); this->notify();
  }
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
typedef smsc::core::buffers::IDAPoolObjRef_T<IAPQueryAC, IAPQueryId> IAPQueryRef;

class IAPQueriesPoolIface {
protected:
  virtual ~IAPQueriesPoolIface()
  { }

public:
  // -------------------------------------------------------
  // -- IAPQueriesPoolIface interface methods
  // -------------------------------------------------------
  virtual IAPQueryRef   allcQuery(void) /*throw()*/ = 0;
  virtual IAPQueryRef   atQuery(IAPQueryId qry_id) /*throw()*/ = 0;
  virtual void          reserveObj(IAPQueryId num_obj) /*throw()*/ = 0;
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_QUERY_DEFS_HPP */

