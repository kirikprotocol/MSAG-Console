#ident "$Id$"
#ifndef SMSC_INMAN_CACHE_DB_HPP
#define SMSC_INMAN_CACHE_DB_HPP

#include <list>
#include <map>

#include "inman/incache.hpp"
#include "core/threads/ThreadPool.hpp"
#include "db/DataSource.h"

using smsc::logger::Logger;
using smsc::core::buffers::Hash;
using smsc::core::threads::Thread;
using smsc::core::threads::ThreadedTask;
using smsc::core::threads::ThreadPool;
using smsc::core::synchronization::Event;

using smsc::db::DataSource;
using smsc::db::Routine;

using smsc::inman::cache::AbonentBillType;
using smsc::inman::cache::InAbonentProviderITF;
using smsc::inman::cache::AbonentCacheITF;
using smsc::inman::cache::InAbonentQueryListenerITF;

namespace smsc {
namespace inman {
namespace cache {
namespace db { 


class AbonentQuery;

class DBQueryManagerITF {
public:
    virtual void releaseQuery(AbonentQuery * query) = 0;
    virtual bool hasListeners(AbonentId & ab_number) = 0;
};

class AbonentQuery : public ThreadedTask {
protected:
    Mutex               mutex;
    DBQueryManagerITF*  _owner;
    DataSource *        _ds;
    const char *        rtId;  //SQL function name
    const char *        rtKey; //SQL function argument name
    std::string         callStr;
    AbonentId           abonent;
    AbonentBillType     abType;
    unsigned            timeOut;
    unsigned            _qId;  //query unique id
    unsigned long       usage; //counter of runs

public:
    AbonentQuery(unsigned q_id, DBQueryManagerITF * owner, DataSource * ds, 
                            //SQL function name and argument name
                            const char * rt_id, const char * rt_key);
    ~AbonentQuery();

    void init(const AbonentId & ab_number, unsigned timeout = 0);

    const AbonentId &     getAbonent(void) const { return abonent; }
    const AbonentBillType getAbonentType(void) const { return  abType; }

    int Execute(void);
    void onRelease(void);
    const char * taskName() { return "AbonentQuery"; }
    void stop();

    unsigned getId(void) const { return _qId; }
    unsigned long Usage(void) const { return usage; }
};


struct DBSourceCFG {
    DataSource *    ds;
    const char *    rtId;
    const char *    rtKey;
    unsigned        max_queries;
    unsigned        init_threads;
    unsigned        timeout;

    DBSourceCFG() {
        ds = NULL; rtId = rtKey = NULL; max_queries = init_threads = timeout = 0;
    }
};

typedef std::list<InAbonentQueryListenerITF *> QueryCBList;

class DBAbonentProvider : public InAbonentProviderITF, public DBQueryManagerITF {
private:
    typedef std::list<AbonentQuery*> QueriesList;
    typedef struct {
        AbonentQuery*   qryDb;
        QueryCBList     cbList;
    } CachedQuery;
    typedef Hash<CachedQuery> QueriesHash;

    Logger *            logger;
    DBSourceCFG         _cfg;
    ThreadPool          pool;

    Mutex               qrsGuard;
    QueriesList         qryPool;
    QueriesHash         qryCache;
    AbonentCacheITF *   cache;
    unsigned            _lastQId;

protected:
    friend class AbonentQuery;
    //DBQueryManagerITF interface methods
    void releaseQuery(AbonentQuery * query);
    bool hasListeners(AbonentId & ab_number);

public:
    DBAbonentProvider(const DBSourceCFG *in_cfg, Logger * uselog = NULL);
    ~DBAbonentProvider();

    // ****************************************
    // InAbonentProviderITF implementation:
    // ****************************************
    void bindCache(AbonentCacheITF * use_cache); 
    //Starts query and binds listener to it. If AbonentCache is bound, the abonent info
    //will be stored in it on query completion. 
    //Returns true if query succesfully started, false otherwise
    bool startQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb = NULL);
    //Unbinds query listener, cancels query if no listeners remain.
    void cancelQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb);
    void cancelAllQueries(void);
};

} //db
} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_DB_HPP */

