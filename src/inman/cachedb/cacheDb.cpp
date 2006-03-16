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
AbonentQuery::AbonentQuery(DBAbonentProvider * owner, DataSource * ds,
                           const char * rt_id, const char * rt_key)
    : ThreadedTask()
    , _owner(owner), _ds(ds), rtId(rt_id), rtKey(rt_key), timeOut(0)
{
    callStr += rtId; callStr += "(:";
    callStr += rtKey; callStr += ");";
}

AbonentQuery::~AbonentQuery()
{
    if (!isReleased)
        onRelease();
}

void AbonentQuery::stop()
{
    MutexGuard tmp(mutex);
    isStopping = true;
}

void AbonentQuery::init(AbonentId ab_number, unsigned timeout/* = 0*/)
{
    MutexGuard tmp(mutex);
    signaled = 0;
    abonent = ab_number;
    abType = btUnknown;
    isReleased = isStopping = false;
    timeOut = timeout;
}

//This one is called by ThreadPool on Exceute() completion
void AbonentQuery::onRelease(void)
{
    MutexGuard tmp(mutex);
    isReleased = true;
    _owner->releaseQuery(this);
}

int AbonentQuery::Execute(void)
{ 
    mutex.Lock();
    if (isStopping) {
        mutex.Unlock();
        return 0;
    }
    mutex.Unlock();
    //sleep(24); //for debugging

    int    status = 0;
    Connection * dcon = _ds->getConnection(); //waits for free connect
    Routine * rtq = NULL;
    int16_t res = -1;

    //may throw SQLException
    try { rtq = dcon->getRoutine(rtId, callStr.c_str(), true); }
    catch (...) { status = -1; }
    if (rtq) {
        try {
            int timerId = -1;
            rtq->setString(rtKey, abonent.c_str());
            if (timeOut)
                timerId = _ds->startTimer(dcon, timeOut);
            rtq->execute(); //make query
            if (timerId >= 0)
                _ds->stopTimer(timerId);
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
    return status;
}

/* ************************************************************************** *
 * class DBAbonentProvider implementation:
 * ************************************************************************** */
DBAbonentProvider::DBAbonentProvider(const DBSourceCFG *in_cfg, Logger * uselog/* = NULL*/)
    : _cfg(*in_cfg)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.cache.db.AbProvider");
    pool.setMaxThreads((int)_cfg.max_queries);
    if (_cfg.init_threads)
        pool.preCreateThreads((int)_cfg.init_threads);
}

DBAbonentProvider::~DBAbonentProvider()
{ 
    MutexGuard  guard(qrsGuard);
    cache = NULL;
    cancelAllQueries();
    pool.shutdown();
    for (QueriesList::iterator it = qryPool.begin(); it != qryPool.end(); it++) {
        delete *it;
    }
    if (qryCache.GetUsage())
        smsc_log_warn(logger, "AbProvider: Queries cache is not empty!");
    qryCache.Empty();
    smsc_log_debug(logger, "AbProvider: shutdown complete");
}

void DBAbonentProvider::cancelAllQueries(void)
{
    MutexGuard  guard(qrsGuard);

    char*       ab_number = NULL; //AbonentId !!!
    CachedQuery *   ab_rec = NULL;

    QueriesHash::Iterator cit = qryCache.getIterator();
    while (cit.Next(ab_number, ab_rec)) {
        smsc_log_debug(logger, "AbProvider: cancelling query(%s):%u",
                       ab_number, ab_rec->cbList.size());
        ab_rec->cbList.clear();
        ab_rec->qryDb->stop();
    }
    return;
}


//This one is called from ThreadedTask on DB query completion.
//Notifies query listeners and releases query.
void DBAbonentProvider::releaseQuery(AbonentQuery * query)
{
    AbonentId       ab_number = query->getAbonent();
    AbonentBillType ab_type = query->getAbonentType();
    QueryCBList     cb_list;

    qrsGuard.Lock();
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number);
    if (ab_rec) {
        if (ab_rec->cbList.size())
            cb_list = ab_rec->cbList;
        qryCache.Delete(ab_number);
    } else
        smsc_log_debug(logger, "AbProvider: no listeners for query(%s)", ab_number);
    qryPool.push_back(query);
    qrsGuard.Unlock();

    if (cache) //update cache
        cache->setAbonentInfo(ab_number, ab_type);
    //notify listeners (if any)
    for (QueryCBList::iterator it = cb_list.begin(); it != cb_list.end(); it++) {
        (*it)->onAbonentQueried(ab_number, ab_type);
    }
    smsc_log_debug(logger, "AbProvider: query(%s) is finished", ab_number);
    return;
}

// ----------------------------------------------
// InAbonentProviderITF interface implementation:
// ----------------------------------------------
void DBAbonentProvider::bindCache(AbonentCacheITF * use_cache)
{
    MutexGuard  guard(qrsGuard);
    cache = use_cache; 
} 
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
//NOTE: the AbonentId is copied into AbonentQuery
bool DBAbonentProvider::startQuery(AbonentId ab_number, 
                                   InAbonentQueryListenerITF * pf_cb/* = NULL*/)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number);
    if (ab_rec) { //allocated query exists, just add callback to list
        ab_rec->cbList.push_back(pf_cb);
        smsc_log_debug(logger, "AbProvider: listener is added to query(%s)", ab_number);
        return true;
    }

    CachedQuery  qryRec;
    if (qryPool.size()) {
        qryRec.qryDb = *(qryPool.begin());
        qryPool.pop_front();
    } else {
        qryRec.qryDb = new AbonentQuery(this, _cfg.ds, _cfg.rtId, _cfg.rtKey);
        if (!qryRec.qryDb)
            return false;
    }
    qryRec.cbList.push_back(pf_cb);
    qryRec.qryDb->init(ab_number, _cfg.timeout);
    qryCache.Insert(ab_number, qryRec);
    pool.startTask(qryRec.qryDb, false); //do not delete task on completion !!!
    smsc_log_debug(logger, "AbProvider: query(%s) is added to queue", ab_number);
    return true;
}

//Unbinds query listener and cancels query
void DBAbonentProvider::cancelQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number);
    if (!ab_rec) {
        smsc_log_warn(logger, "AbProvider: attempt to cancel unexisting query!");
        return;
    }
    smsc_log_debug(logger, "AbProvider: cancelling query(%s)", ab_number);

    QueryCBList::iterator it = std::find(ab_rec->cbList.begin(),
                                         ab_rec->cbList.end(), pf_cb);
    if (it != ab_rec->cbList.end()) {
        ab_rec->cbList.erase(it);
        if (!ab_rec->cbList.size()) { //cancel DB query
            ab_rec->qryDb->stop();
            qryCache.Delete(ab_number);
        } else
            smsc_log_debug(logger, "AbProvider: %u listeners remain for query(%s)",
                           ab_rec->cbList.size(), ab_number);
    }
    return;
}

} //db
} //cache
} //inman
} //smsc



