static const char ident[] = "$Id$";

#include "inman/abprov/facility/IAPThrFacility.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPQueryAC implementation:
 * ************************************************************************** */
IAPQueryAC::IAPQueryAC(unsigned q_id, IAPQueryManagerITF * owner,
                       unsigned timeout_secs, Logger * use_log/* = NULL*/)
    : ThreadedTask(), _qId(q_id), _owner(owner)
    , timeOut(timeout_secs), usage(0), logger(use_log)
{
}

IAPQueryAC::~IAPQueryAC()
{
    MutexGuard tmp(_mutex);
    if (!isReleased) {
        isReleased = true;
        _owner->releaseQuery(this);
    }
}

bool IAPQueryAC::init(const AbonentId & ab_number)
{
    MutexGuard tmp(_mutex);
    abonent = ab_number;
    abRec.reset();
    isReleased = isStopping = false;
    usage++;
    return true;
}

//This one is called by ThreadPool on Exceute() completion
void IAPQueryAC::onRelease(void)
{
    MutexGuard tmp(_mutex);
    isReleased = true;
    _owner->releaseQuery(this);
}

/* ************************************************************************** *
 * class IAProviderThreaded implementation:
 * ************************************************************************** */
IAProviderThreaded::IAProviderThreaded(const IAProviderThreadedCFG & in_cfg, Logger * uselog/* = NULL*/)
    : _cfg(in_cfg), _lastQId(0)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.abprov.db");
    pool.setMaxThreads((int)_cfg.max_queries);
    if (_cfg.init_threads)
        pool.preCreateThreads((int)_cfg.init_threads);
}

IAProviderThreaded::~IAProviderThreaded()
{ 
    cache = NULL;
    cancelAllQueries();
    pool.shutdown(); //waits or kills threads
    for (QueriesList::iterator it = qryPool.begin(); it != qryPool.end(); it++) {
        delete *it;
    }
    if (qryCache.GetUsage())
        smsc_log_warn(logger, "IAPrvd: shutdown - queries cache is not empty!");
    qryCache.Empty();
    smsc_log_debug(logger, "IAPrvd: shutdown complete");
}

void IAProviderThreaded::cancelAllQueries(void)
{
    MutexGuard  guard(qrsGuard);

    char*       ab_number = NULL; //AbonentId.value !!!
    CachedQuery *   ab_rec = NULL;

    QueriesHash::Iterator cit = qryCache.getIterator();
    while (cit.Next(ab_number, ab_rec)) {
        smsc_log_debug(logger, "IAPrvd: cancelling query[%u:%lu](%s):%u",
                        ab_rec->query->getId(), ab_rec->query->Usage(), 
                       ab_number, ab_rec->cbList.size());
        ab_rec->cbList.clear();
        ab_rec->query->stop();
    }
    return;
}


bool IAProviderThreaded::hasListeners(const AbonentId & ab_number)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number.getSignals());
    return (ab_rec && ab_rec->cbList.size()) ? true : false;
}

//This one is called from ThreadedTask on DB query completion.
//Notifies query listeners and releases query.
void IAProviderThreaded::releaseQuery(IAPQueryAC * query)
{
    MutexGuard guard(qrsGuard);
    AbonentId       ab_number = query->getAbonentId();
    AbonentRecord   abRec = query->getAbonentRecord();
    CachedQuery *   qry_rec = qryCache.GetPtr(ab_number.getSignals());

    if (qry_rec) {
        if (qry_rec->cbList.size()) { //query wasn't cancelled
            if (cache) //update cache
                cache->setAbonentInfo(ab_number, &abRec);
            //notify listeners
            for (QueryCBList::iterator it = qry_rec->cbList.begin();
                                        it != qry_rec->cbList.end(); it++)
                (*it)->onIAPQueried(ab_number, abRec.ab_type, abRec.getSCFinfo());
        } else
            smsc_log_debug(logger, "IAPrvd: no listeners for query[%u:%lu](%s)",
                           query->getId(), query->Usage(), ab_number.getSignals());

        qryCache.Delete(ab_number.getSignals());
    } else
        smsc_log_warn(logger, "IAPrvd: not in cache query[%u:%lu](%s)",
                       query->getId(), query->Usage(), ab_number.getSignals());

    if (!query->delOnCompletion())
        qryPool.push_back(query);
    //else query is being deleted by PooledThread::Execute()
    smsc_log_debug(logger, "IAPrvd: query[%u:%lu](%s) is finished, type: %s (%u)",
                   query->getId(), query->Usage(), ab_number.getSignals(),
                   abRec.type2Str(), abRec.ab_type);
    return;
}

// ----------------------------------------------
// InAbonentProviderITF interface implementation:
// ----------------------------------------------
void IAProviderThreaded::bindCache(AbonentCacheITF * use_cache)
{
    MutexGuard  guard(qrsGuard);
    cache = use_cache; 
} 
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
//NOTE: the AbonentId is copied into AbonentQuery
bool IAProviderThreaded::startQuery(const AbonentId & ab_number, 
                                   IAPQueryListenerITF * pf_cb/* = NULL*/)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * qry_rec = qryCache.GetPtr(ab_number.getSignals());
    if (qry_rec) { //allocated query exists, just add callback to list
        qry_rec->cbList.push_back(pf_cb);
        smsc_log_debug(logger, "IAPrvd: listener is added to query[%u:%lu](%s)",
                       qry_rec->query->getId(), qry_rec->query->Usage(),
                       ab_number.getSignals());
        return true;
    }

    CachedQuery  qryRec;
    if (qryPool.size()) {
        qryRec.query = *(qryPool.begin());
        qryPool.pop_front();
    } else {
        _lastQId++;
        qryRec.query = _cfg.qryPlant->newQuery(_lastQId, this, logger);
        if (!qryRec.query)
            return false;
    }
    qryRec.cbList.push_back(pf_cb);
    if (qryRec.query->init(ab_number)) {
        qryCache.Insert(ab_number.getSignals(), qryRec);
        pool.startTask(qryRec.query, !_cfg.qryMultiRun); //do not delete task on completion !!!
        smsc_log_debug(logger, "IAPrvd: query[%u:%lu](%s) is added to queue",
                       qryRec.query->getId(), qryRec.query->Usage(),
                       ab_number.getSignals());
    } else
        smsc_log_error(logger, "IAPrvd: query[%u:%lu](%s) failed to init",
                       qryRec.query->getId(), qryRec.query->Usage(),
                       ab_number.getSignals());
    return true;
}

//Unbinds query listener and cancels query
void IAProviderThreaded::cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF * pf_cb)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * qry_rec = qryCache.GetPtr(ab_number.getSignals());
    if (!qry_rec) {
        smsc_log_warn(logger, "IAPrvd: attempt to cancel unexisting query!");
        return;
    }
    smsc_log_debug(logger, "IAPrvd: cancelling query[%u:%lu](%s)",
                    qry_rec->query->getId(), qry_rec->query->Usage(),
                    ab_number.getSignals());

    QueryCBList::iterator it = std::find(qry_rec->cbList.begin(),
                                         qry_rec->cbList.end(), pf_cb);
    if (it != qry_rec->cbList.end())
        qry_rec->cbList.erase(it);
    if (qry_rec->cbList.size())
        smsc_log_debug(logger, "IAPrvd: %u listeners remain for query[%u:%lu](%s)",
                       qry_rec->cbList.size(), qry_rec->query->getId(),
                       qry_rec->query->Usage(), ab_number.getSignals());
    return;
}

} //iaprvd
} //inman
} //smsc


