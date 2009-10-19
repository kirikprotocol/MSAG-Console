/* ************************************************************************** *
 * Threaded Abonent Provider facility.
 * In order to use IAProviderFacility functionlity implement IAPQueryAC and
 * IAPQueryFactoryITF.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_THREADED_FACILITY_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_IAP_THREADED_FACILITY_HPP

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
protected:
    EventMonitor        _mutex;
    IAPQueryManagerITF* _owner;
    unsigned long       usage;  //counter of runs
    unsigned            timeOut;
    unsigned            _qId;   //query unique id
    AbonentId           abonent;
    AbonentSubscription abInfo;
    Logger *            logger;
    std::string         tName;  //task name for logging
    IAPQStatus::Code    _qStatus;   //query completion status, is to return by Execute()
    RCHash              _qError;
    std::string         _exc;   //query error/exception message

    //Composes taskName, it's recommended to call it in successors constructor
    void mkTaskName(void)
    {
        char buf[sizeof("[%u:%lu]") + sizeof(_qId)*3 + sizeof(usage)*3];
        snprintf(buf, sizeof(buf)-1, "[%u:%lu]", _qId, usage);
        tName = taskType();
        tName += buf;
    }

public:
    IAPQueryAC(unsigned q_id, IAPQueryManagerITF * owner,
               unsigned timeout_secs, Logger * use_log = NULL);
    virtual ~IAPQueryAC();

    //NOTE: successor may maintain his own init(), but nevertheless it must call this one
    virtual bool init(const AbonentId & ab_number);

    //-- Are to implement:
    virtual const char * taskType(void) const = 0;
    //virtual int Execute(void) = 0;

    const char *             taskName(void)                { return tName.c_str(); }
    const AbonentSubscription & getAbonentInfo(void) const { return abInfo; }
    const AbonentId &        getAbonentId(void)      const { return abonent; }
    unsigned                 getId(void)             const { return _qId; }
    unsigned long            Usage(void)             const { return usage; }
    IAPQStatus::Code         Status(void)            const { return _qStatus; }
    RCHash                   Error(void)             const { return _qError; }
    std::string                     Status2Str(void)        const;

protected:
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
    unsigned            _lastQId;
    IAPFacilityCFG      _cfg;
    Logger *            logger;
    const char *        _logId; //prefix for logging info

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

