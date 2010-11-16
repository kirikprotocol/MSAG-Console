/* ************************************************************************** *
 * Threaded Abonent Provider facility.
 * In order to use IAProviderFacility functionlity implement IAPQueryAC and
 * IAPQueryFactoryITF.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_THREADED_FACILITY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_THREADED_FACILITY_HPP

#include <list>

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::core::threads::ThreadedTask;
using smsc::core::threads::ThreadPool;
using smsc::core::synchronization::EventMonitor;

using smsc::logger::Logger;

/* ************************************************************************** *
 * AbonentProvider as a Thread Pool of IAPQueriesAC implementations:
 * ************************************************************************** */

class IAPQueryAC;

class IAPQueryManagerITF {
protected:
  virtual ~IAPQueryManagerITF() //forbid interface destruction
  { }

public:
  virtual void releaseQuery(IAPQueryAC * query) = 0;
  virtual bool hasListeners(const AbonentId & ab_number) = 0;
};


class IAPQueryFactoryITF {
protected:
  virtual ~IAPQueryFactoryITF()  //forbid interface destruction
  { }

public:
  virtual IAPQueryAC * newQuery(unsigned q_id, IAPQueryManagerITF * owner,
                                Logger * use_log = NULL) = 0;
};

class IAPQueryAC : public ThreadedTask {
public:
  static const unsigned _nmTypeSZ = 32;
  static const unsigned _idTaskSZ = (unsigned)(sizeof("[%u:%lu]")
                                               + sizeof(unsigned /*_qId*/)*3
                                               + sizeof(unsigned long/*usage*/)*3);
  static const unsigned _nmQuerySZ = _idTaskSZ + _nmTypeSZ;

  typedef core::buffers::FixedLengthString<_nmTypeSZ>  TypeString_t;
  typedef core::buffers::FixedLengthString<_nmQuerySZ>  QueryName_t;

  IAPQueryAC(unsigned q_id, IAPQueryManagerITF * owner,
             unsigned timeout_secs, Logger * use_log = NULL);
  virtual ~IAPQueryAC();

  //NOTE: successor may maintain his own init(), but nevertheless it must call this one
  virtual bool init(const AbonentId & ab_number);

  //-- Are to implement:
  virtual const TypeString_t & taskType(void) const = 0;
  //virtual int Execute(void) = 0;

  // -------------------------------------------
  // -- ThreadedTask interface methods
  // -------------------------------------------
  const char *  taskName(void) { return tName.c_str(); }

  const AbonentSubscription & getAbonentInfo(void) const { return abInfo; }
  const AbonentId &        getAbonentId(void)      const { return abonent; }
  unsigned                 getId(void)             const { return _qId; }
  unsigned long            Usage(void)             const { return usage; }
  IAPQStatus::Code         Status(void)            const { return _qStatus; }
  RCHash                   getError(void)          const;
  std::string              status2Str(void)        const;

protected:
  EventMonitor        _mutex;
  unsigned            _qId;   //query unique id
  IAPQueryManagerITF* _owner;
  unsigned long       usage;  //counter of runs
  unsigned            timeOut;
  AbonentId           abonent;
  AbonentSubscription abInfo;
  QueryName_t         tName;  //task name for logging
  IAPQStatus::Code    _qStatus;   //query completion status, is to return by Execute()
  RCHash              _qError;
  std::string         _exc;   //query error/exception message
  Logger *            logger;

  //Composes taskName, it's recommended to call it in successors constructor
  void mkTaskName(void);

  /* -- */
  friend class smsc::core::threads::ThreadPool;
  void onRelease(void);
};


struct IAPFacilityCFG {
  unsigned            max_queries;
  unsigned            init_threads;
  bool                qryMultiRun;
  IAPQueryFactoryITF * qryPlant;
};


class IAPQueryFacility : public IAPQueryProcessorITF, IAPQueryManagerITF {
private:
  typedef std::list<IAPQueryListenerITF*> QueryCBList;
  typedef std::list<IAPQueryAC*> QueriesList;
  struct CachedQuery {
      IAPQueryAC *    query;
      QueryCBList     cbList;
  };
  typedef Hash<CachedQuery> QueriesHash;

  ThreadPool          pool;
  Mutex               qrsGuard;
  QueriesList         qryPool;
  QueriesHash         qryCache;
  IAPFacilityCFG      _cfg;
  unsigned            _lastQId;
  const char *        _logId; //prefix for logging info
  Logger *            logger;

protected:
  friend class IAPQueryAC;
  //IAPQueryManagerITF interface methods
  void releaseQuery(IAPQueryAC * query);
  bool hasListeners(const AbonentId & ab_number);

public:
  IAPQueryFacility(const IAPFacilityCFG & in_cfg, Logger * use_log = NULL);
  ~IAPQueryFacility();

  bool Start(void);
  void Stop(bool do_wait = false);

  // ****************************************
  // IAPQueryProcessorITF implementation:
  // ****************************************
  //Starts query and binds listener to it.
  //Returns true if query succesfully started, false otherwise
  bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb);
  //Unbinds query listener, cancels query if no listeners remain.
  void cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb);
  void cancelAllQueries(void);
};


} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_THREADED_FACILITY_HPP */

