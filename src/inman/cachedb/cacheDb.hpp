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
using smsc::inman::cache::InAbonentQueryManagerITF;
using smsc::inman::cache::AbonentCacheITF;
using smsc::inman::cache::InAbonentQueryListenerITF;

namespace smsc {
namespace inman {
namespace cache {
namespace db { 


class DBQueryManager;

class AbonentQuery : public ThreadedTask, public Event {
protected:
    DBQueryManager *    _owner;
    DataSource *        _ds;
    const char *        rtId;  //SQL function name
    const char *        rtKey; //SQL function argument name
    std::string         callStr;
    AbonentId           abonent;
    AbonentBillType     abType;
    InAbonentQueryListenerITF * _fcbDone;

public:
    AbonentQuery(DBQueryManager * owner, DataSource * ds, 
                            const char * rt_id, const char * rt_key);
    ~AbonentQuery();

    void init(AbonentId ab_number, InAbonentQueryListenerITF * fcb_done);

    AbonentId   getAbonent(void) const { return  abonent; }
    const AbonentBillType getAbonentType(void) const { return  abType; }

    int Execute(void);
    void onRelease(void);
    const char * taskName() { return "AbonentQuery"; }
};


struct DBSourceCFG {
    DataSource *    ds;
    const char *    rtId;
    const char *    rtKey;
    unsigned        max_queries;
    unsigned        init_threads;

    DBSourceCFG() { ds = NULL; rtId = rtKey = NULL; max_queries = init_threads = 0; }
};


class DBQueryManager : public InAbonentQueryManagerITF {
private:
    typedef std::list<AbonentQuery*> QueriesList;
    typedef std::map<AbonentId, AbonentQuery*> QueriesMap;

    Logger *        logger;
    DBSourceCFG    _cfg;
    ThreadPool      pool;

    Mutex           qrsGuard;
    QueriesMap      queries;
    QueriesList     freeQueries;

protected:
    friend class AbonentQuery;
    void releaseQuery(AbonentQuery * query);
    AbonentQuery * initQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb = NULL);

public:
    DBQueryManager(const DBSourceCFG *in_cfg, Logger * uselog = NULL);
    ~DBQueryManager();

    // ****************************************
    // InAbonentQueryManagerITF implementation:
    // ****************************************
    bool isQuering(AbonentId ab_number);
    bool startQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb = NULL);
    int  cancelQuery(AbonentId ab_number, bool wait = false);
    int  execQuery(AbonentId ab_number, AbonentBillType & result,
                                             unsigned short timeout_secs = 0);
};


struct AbonentRecord {
    typedef std::list<InAbonentQueryListenerITF *> CallBacksList;

    AbonentBillType     ab_type;
    time_t          tm_expired;
    CallBacksList   cb_list; //functions to call on info update

    AbonentRecord()
        : ab_type(smsc::inman::cache::btUnknown), tm_expired(0) {}
    AbonentRecord(AbonentBillType type, time_t tmex)
        : ab_type(type), tm_expired(tmex) {}
};

class AbonentCacheDB : public AbonentCacheITF, InAbonentQueryListenerITF {
protected:

    Logger *              logger;
    time_t                cacheInterval;
    DBQueryManager        *qMgr;
    Mutex                 cacheGuard;
    Hash<AbonentRecord>   cache;

    // ****************************************
    // InAbonentQueryListenerITF implementation:
    // ****************************************

    //NOTE: this callback is called from ThreadedTask, on calling the AbonentRecord
    //always exists for given ab_number
    void abonentQueryCB(AbonentId ab_number, AbonentBillType ab_type);

public:
    AbonentCacheDB(DBSourceCFG * cfg, time_t cache_interval, Logger * uselog = NULL);
    ~AbonentCacheDB();

    // ****************************************
    // AbonentCacheITF implementation:
    // ****************************************
    void setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type, time_t expired = 0);
    AbonentBillType getAbonentInfo(AbonentId ab_number);

    //deprecated!!!
    //starts thread updating AbonentInfo and waits for its completion
    AbonentBillType waitAbonentInfoUpdate(AbonentId ab_number, unsigned short timeout_secs = 0);

    //starts thread updating AbonentInfo, that will call the callback upon completion
    bool queryAbonentInfo(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb);
};

} //db
} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_DB_HPP */

