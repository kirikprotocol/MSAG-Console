static const char ident[] = "$Id$";

#include "inman/cachedb/cacheDb.hpp"

using smsc::db::Connection;

namespace smsc {
namespace inman {
namespace cache {
namespace db { 

/* ************************************************************************** *
 * class AbonentQuery implementation:
 * ************************************************************************** */
AbonentQuery::AbonentQuery(DBQueryManager * owner, DataSource * ds,
                           const char * rt_id, const char * rt_key)
    : ThreadedTask()
    , _owner(owner), _ds(ds), rtId(rt_id), rtKey(rt_key), _fcbDone(NULL)
{
    callStr += rtId; callStr += "(:";
    callStr += rt_key; callStr += ");";
}

AbonentQuery::~AbonentQuery()
{
    if (!isReleased)
        onRelease();
}

void AbonentQuery::init(AbonentId ab_number, InAbonentQueryListenerITF * fcb_done)
{
    mutex.Lock();
    signaled = 0;
    abonent = ab_number;
    abType = btUnknown;
    _fcbDone = fcb_done;
    isReleased = false;
    mutex.Unlock();
}
void AbonentQuery::onRelease(void)
{
    isReleased = true;
    _owner->releaseQuery(this);
    this->Signal();
}

int AbonentQuery::Execute(void)
{ 
    int    status = 0;
    Connection * dcon = _ds->getConnection(); //waits for free connect
    Routine * rtq = NULL;
    int16_t res = -1;

    //may throw SQLException
    try { rtq = dcon->getRoutine(rtId, callStr.c_str(), true); }
    catch (...) { status = -1; }
    if (rtq) {
        try {
            rtq->setString(rtKey, abonent);
            rtq->execute();
            res = rtq->getInt16("RETURN"); //FUNCTION_RETURN_ATTR_NAME
        } catch (Exception& exc) {
            status = -2; 
        }
    }
    //convert returned integer to AbonentBillType value
    if (res > 0) 
        abType = btPrepaid;
    else if (!res)
        abType = btPostpaid;
    //else btUnknown

    //do not unregister routine in order to reuse it in next queries.
    //routine will be deleted by ~DataSource().
    _ds->freeConnection(dcon);

    if (!isStopping)
        _fcbDone->abonentQueryCB(abonent, abType);
    return status;
}

/* ************************************************************************** *
 * class DBQueryManager implementation:
 * ************************************************************************** */
DBQueryManager::DBQueryManager(const DBSourceCFG *in_cfg, Logger * uselog/* = NULL*/)
    : _cfg(*in_cfg), logger(uselog)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.cache.db.QueryMgr");

    pool.setMaxThreads((int)_cfg.max_queries);
    if (_cfg.init_threads)
        pool.preCreateThreads((int)_cfg.init_threads);
}
DBQueryManager::~DBQueryManager()
{ 
    pool.shutdown();
    QueriesMap::iterator it = queries.begin();
    for (; it != queries.end(); it++) {
        delete (*it).second;
    }
}

//private:
void DBQueryManager::releaseQuery(AbonentQuery * query)
{
    MutexGuard  guard(qrsGuard);
    freeQueries.push_back(query);
    QueriesMap::iterator it = queries.find(query->getAbonent());
    if (it != queries.end())
        queries.erase(it);
    else
        smsc_log_error(logger, "QueryMgr: releasing unregistered query ..");
}

AbonentQuery * DBQueryManager::initQuery(AbonentId ab_number,
                                         InAbonentQueryListenerITF * pf_cb/* = NULL*/)
{
    MutexGuard  guard(qrsGuard);
    AbonentQuery * ntask;
    if (freeQueries.size()) {
        QueriesList::iterator it = freeQueries.begin();
        ntask = (*it);
        freeQueries.erase(it);
    } else {
        ntask = new AbonentQuery(this, _cfg.ds, _cfg.rtId, _cfg.rtKey);
        if (!ntask)
            return NULL;
    }
    queries.insert(QueriesMap::value_type(ab_number, ntask));
    ntask->init(ab_number, pf_cb);
    return ntask;
}

// ****************************************
// InAbonentQueryManagerITF implementation:
// ****************************************
bool DBQueryManager::isQuering(AbonentId ab_number)
{
    MutexGuard  guard(qrsGuard);
    QueriesMap::iterator it = queries.find(ab_number);
    return (it != queries.end());
}
bool DBQueryManager::startQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb/* = NULL*/)
{
    AbonentQuery * ntask = initQuery(ab_number, pf_cb);
    if (!ntask)
        return false;
    smsc_log_debug(logger, "QueryMgr: quering abonent %s info", ab_number);
    pool.startTask(ntask, false); //do not delete task on completion !!!
    return true;
}
int DBQueryManager::cancelQuery(AbonentId ab_number, bool wait/* = false*/)
{
    MutexGuard  guard(qrsGuard);
    QueriesMap::iterator it = queries.find(ab_number);
    if (it != queries.end()) {
        AbonentQuery * query = (*it).second;
        query->stop();
        if (wait)
            return query->Wait();
    } else
        return (-1);
    return ETIMEDOUT;
}
int DBQueryManager::execQuery(AbonentId ab_number, AbonentBillType & result,
                              unsigned short timeout_secs/* = 0*/)
{ 
    AbonentQuery * ntask = initQuery(ab_number, NULL);
    if (!ntask)
        return (-1);
    pool.startTask(ntask, false); //do not delete task on completion !!!
    int rval = timeout_secs ? ntask->Wait(1000*timeout_secs) : ntask->Wait();
    result = ntask->getAbonentType();
    return rval;
}


/* ************************************************************************** *
 * class AbonentCacheDB implementation:
 * ************************************************************************** */
AbonentCacheDB::AbonentCacheDB(DBSourceCFG * cfg, time_t cache_interval,
                               Logger * uselog/* = NULL*/)
    : cacheInterval(cache_interval), logger(uselog)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.db.Cache");
    qMgr = new DBQueryManager(cfg, uselog);
}

AbonentCacheDB::~AbonentCacheDB()
{
    delete qMgr;
}


//NOTE: This is cache synchronization callback function. It's called from ThreadedTask.
void AbonentCacheDB::abonentQueryCB(AbonentId ab_number, AbonentBillType ab_type)
{
    cacheGuard.Lock();
    AbonentRecord * ab_rec = cache.GetPtr(ab_number);
    if (!ab_rec) {
        smsc_log_error(logger, "dbCache: cache inconsistent, abonent %s!", ab_number);
        //create dummy record
        AbonentRecord rec(btUnknown, time(NULL));
        ab_rec = cache.SetItem(ab_number, rec);
    }
    ab_rec->ab_type = ab_type;
    ab_rec->tm_expired = time(NULL) + cacheInterval;
    
    AbonentRecord::CallBacksList cb_list = ab_rec->cb_list;
    ab_rec->cb_list.clear();
    cacheGuard.Unlock();
    smsc_log_debug(logger, "dbCache: abonent %s info is updated", ab_number);

    AbonentRecord::CallBacksList::iterator it = cb_list.begin();
    for (; it != cb_list.end(); it++) {
        InAbonentQueryListenerITF * pf_cb = (*it);
        pf_cb->abonentQueryCB(ab_number, ab_type);
    }
    return;
}

// ****************************************
// AbonentCacheITF implementation:
// ****************************************
void AbonentCacheDB::setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type,
                                    time_t expired /*= 0*/)
{
    if (!expired)
        expired = time(NULL) + cacheInterval;

    cacheGuard.Lock();
    AbonentRecord ab_rec(ab_type, expired);
    int status = cache.Insert(ab_number, ab_rec);
    cacheGuard.Unlock();
    smsc_log_debug(logger, "dbCache: abonent %s info is %s",
                   ab_number, status ? "added" : "updated");
}

AbonentBillType AbonentCacheDB::getAbonentInfo(AbonentId ab_number)
{
    MutexGuard  guard(cacheGuard);
    AbonentRecord * ab_rec = cache.GetPtr(ab_number);
    if (ab_rec && (ab_rec->ab_type != btUnknown)) {
        if (time(NULL) < ab_rec->tm_expired)
            return ab_rec->ab_type;

        smsc_log_debug(logger, "dbCache: abonent %s info is expired", ab_number);
        ab_rec->ab_type = btUnknown; //expired
    }
    return btUnknown;
}

//deprecated!!!
//starts thread updating AbonentInfo and waits for its completion
AbonentBillType AbonentCacheDB::waitAbonentInfoUpdate(AbonentId ab_number,
                                                      unsigned short timeout_secs/* = 0*/)
{
    AbonentBillType ab_type = btUnknown;
    qMgr->execQuery(ab_number, ab_type, timeout_secs);
    setAbonentInfo(ab_number, ab_type, time(NULL) + cacheInterval);
    return ab_type;
}

//starts thread updating AbonentInfo, that will call the callback upon completion
bool AbonentCacheDB::queryAbonentInfo(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb)
{
    MutexGuard  guard(cacheGuard);
    AbonentRecord * ab_rec = cache.GetPtr(ab_number);
    if (!ab_rec) { //create dummy record to store callback function
        AbonentRecord rec(btUnknown, time(NULL));
        ab_rec = cache.SetItem(ab_number, rec);
    }
    if (!ab_rec->cb_list.size()) {
        //static_cast<InAbonentQueryListenerITF*>(this)
        if (!qMgr->startQuery(ab_number, this))
            return false;
    } //already quering, just add listener
    ab_rec->cb_list.push_back(pf_cb);
    return true;
}

} //db
} //cache
} //inman
} //smsc



