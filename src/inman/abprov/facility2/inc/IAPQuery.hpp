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
#include "core/synchronization/EventMonitor.hpp"

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
  //Returns false if event cann't be processed by referee
  virtual bool onQueryEvent(AbonentId ab_id) = 0;
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueryAC : /* public IndexedObj_T<IAPQueryId>, */
                    public smsc::core::synchronization::EventMonitor {
protected:
  explicit IAPQueryAC(IAPQueryId q_id);
  virtual ~IAPQueryAC()  { }

public:
  enum ProcStage_e {
      qryIdle = 0x0       //query is not initialized
    , qryStarted = 0x01   //query is initialized and started
    , qryResulted = 0x02  //query received a result
    , qryReporting = 0x04 //query is reporting its result to listeners
    , qryStopping = 0x08  //query is releasing active resources (connections, etc)
    , qryDone = 0x10      //query finished its processing cycle
                          //(NOTE: if query was cancelled result may not be valid)
  };

  struct Stages {
    ProcStage_e _current;
    uint16_t    _mask;

    Stages() : _current(qryIdle), _mask(0)
    { }

    void clear(void)
    {
      _mask = 0; _current = qryIdle;
    }
    void set(ProcStage_e fsm_st)
    {
      _mask |= (_current = fsm_st); 
    }
    void rollback(ProcStage_e fsm_to, ProcStage_e fsm_old)
    {
      _mask &= ~fsm_old; set(fsm_to);
    }
  };

  static const unsigned _nmTypeSZ = 32;
  static const unsigned _idTaskSZ = (unsigned)(sizeof("[%u:%lu]")
                                               + sizeof(IAPQueryId /*_qId*/)*3
                                               + sizeof(unsigned long/*usage*/)*3);
  static const unsigned _nmQuerySZ = _idTaskSZ + _nmTypeSZ;

  typedef core::buffers::FixedLengthString<_nmTypeSZ + 1>  TypeString_t;
  typedef core::buffers::FixedLengthString<_nmQuerySZ + 1>  QueryName_t;


  // -----------------------------------------------
  // -- IndexedObj_T<IAPQueryId> interface methods
  // -----------------------------------------------
  IAPQueryId getIndex(void) const { return _qId; }

  //assigns abonentId and switches FSM to qryIdle state
  void init(IAPQueryRefereeIface & use_owner, const AbonentId & ab_number);
  //Returns:
  // true  if listener is registered and will be notified as query yelds result.
  // false if query is already completed, so it should be rereported in order to
  //       notify listener. In that case FSM is switched to qryReporting state.
  bool addListener(IAPQueryListenerITF & pf_cb);
  //Removes given listener from query listeners list.
  //Returns true on success, false if listener is already targeted and query
  //waits for its mutex.
  bool removeListener(IAPQueryListenerITF & pf_cb);
  //Reports query to all enqueued listeners, upon completion
  //switches FSM to qryDone state.
  void notifyListeners(void) /*throw()*/;

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
  //otherwise to qryDone state.
  virtual IAPQStatus::Code  start(Logger * use_log = NULL) /*throw()*/ = 0;
  //Cancels query execution, tries to switche FSM to qryStopping/qryDone state.
  //Note: Listeners aren't notified.
  //Returns false if listener is already targeted and query
  //waits for its mutex.
  virtual bool              cancel(void) /*throw()*/ = 0;
  //Returns true if query object may be released.
  //Note: should be at least equal to isCompleted()
  virtual bool              isToRelease(void) /*throw()*/ = 0;
  //Releases all used resources. May be called only at qryDone state.
  //Switches FSM to qryIdle state.
  virtual void              cleanup(void) /*throw()*/ = 0;

private:
  struct ListenerInfo {
    bool                  _isAimed;
    IAPQueryListenerITF * _ptr;
    
    explicit ListenerInfo(IAPQueryListenerITF * use_ptr = NULL)
      : _isAimed(false), _ptr(use_ptr)
    { }
  };
  typedef std::list<ListenerInfo> QueryListeners;

  const IAPQueryId        _qId;     //query unique id
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
    _stages.set(fsm_st); notify(); 
  }
};

/* ************************************************************************** *
 * 
 * ************************************************************************** */
class IAPQueriesPoolIface {
protected:
  virtual ~IAPQueriesPoolIface()
  { }

public:
  virtual void          reserveObj(IAPQueryId num_obj) /*throw()*/ = 0;
  virtual IAPQueryAC *  allcQuery(void) /*throw()*/ = 0;
  virtual void          rlseQuery(IAPQueryAC & use_obj) /*throw()*/ = 0;
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_QUERY_DEFS_HPP */

