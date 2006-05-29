static const char ident[] = "$Id$";

#include "inman/abprov/db/ProviderDb.hpp"
using smsc::inman::cache::_sabBillType;
using smsc::db::Connection;

#include "db/DataSourceLoader.h"
using smsc::db::DataSourceLoader;
using smsc::db::DataSourceFactory;

static const char _provIdent[] = "abProv_DB";

static struct _DS_PARMS {
    const char* prmId;
    char*       prmVal;
}   _dsParm[3] = {
     { "type", NULL }
    ,{ "abonentQueryFunc", NULL }
    ,{ "queryFuncArg", NULL }
};
#define  _DS_IDENT_VAL  _dsParm[0].prmVal

namespace smsc {
namespace inman {
namespace abprov {
namespace db { 

//This is the DB Provider dynamic library entry point
extern "C" AbonentProviderCreatorITF * 
    loadupAbonentProvider(ConfigView* dbCfg, Logger * use_log) throw(ConfigException)
{
    //load drivers first, subsection: DataSourceDrivers
    if (!dbCfg->findSubSection("DataSourceDrivers"))
        throw ConfigException("'DataSourceDrivers' subsection is missed");
    try { DataSourceLoader::loadup(dbCfg);
    } catch (Exception& exc) {  //ConfigException or LoadupException
        throw ConfigException(exc.what());
    }

    //read DB connection parameters
    if (!dbCfg->findSubSection("DataSource"))
        throw ConfigException("'DataSource' subsection is missed");
    ConfigView* dsCfg = dbCfg->getSubConfig("DataSource");
    //
    for (int i = 0; i < 3; i++) {
        if (!(_dsParm[i].prmVal = dsCfg->getString(_dsParm[i].prmId)))
            throw ConfigException("'DataSource.%s' isn't set!", _dsParm[i].prmId);
    }

    DBSourceCFG  dbProvPrm;
    dbProvPrm.rtId = _dsParm[1].prmVal;
    dbProvPrm.rtKey = _dsParm[2].prmVal;

    dbProvPrm.ds = DataSourceFactory::getDataSource(_DS_IDENT_VAL);
    if (!dbProvPrm.ds)
        throw ConfigException("'%s' 'DataSource' isn't registered!", _DS_IDENT_VAL);
    dbProvPrm.ds->init(dsCfg); //throws

    dbProvPrm.max_queries = (unsigned)dsCfg->getInt("maxQueries"); //throws
    dbProvPrm.init_threads = 1;
    bool wdog = false;
    try { wdog = dsCfg->getBool("watchdog"); }
    catch (ConfigException & exc) { }

    if (wdog)
        dbProvPrm.timeout = (unsigned)dsCfg->getInt("timeout"); //throws
    return new DBAbonentProviderCreator(dbProvPrm);
}

/* ************************************************************************** *
 * class DBAbonentProviderCreator implementation:
 * ************************************************************************** */
DBAbonentProviderCreator::~DBAbonentProviderCreator()
{
    ProvidersLIST::iterator it = providers.begin();
    for (; it != providers.end(); it++)
        delete (*it);
    providers.clear();

    if (cfg.ds)
        delete cfg.ds;
}

const char * DBAbonentProviderCreator::ident(void) const
{
    return _provIdent;
}

InAbonentProviderITF * DBAbonentProviderCreator::create(Logger * use_log)
{
    return new DBAbonentProvider(&cfg, use_log);
}

void  DBAbonentProviderCreator::logConfig(Logger * use_log) const
{
    smsc_log_info(use_log, "FUNCTION %s(%s IN VARCHAR)", cfg.rtId, cfg.rtKey);
    smsc_log_info(use_log, "Max.queries: %u", cfg.max_queries);
    smsc_log_info(use_log, "Query timeout: %u secs", cfg.timeout);
}
/* ************************************************************************** *
 * class AbonentQuery implementation:
 * ************************************************************************** */
AbonentQuery::AbonentQuery(unsigned q_id, DBQueryManagerITF * owner, DataSource * ds,
                           const char * rt_id, const char * rt_key)
    : ThreadedTask(), _qId(q_id)
    , _owner(owner), _ds(ds), rtId(rt_id), rtKey(rt_key), timeOut(0), usage(0)
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

void AbonentQuery::init(const AbonentId & ab_number, unsigned timeout/* = 0*/)
{
    MutexGuard tmp(mutex);
    abonent = ab_number;
    abType = smsc::inman::cache::btUnknown;
    isReleased = isStopping = false;
    timeOut = timeout;
    usage++;
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
    {
        MutexGuard tmp(mutex);
        if (isStopping || !_owner->hasListeners(abonent))
            return 0; //query was cancelled by either QueryManager or ThreadPool
    }
    //sleep(24); //for debugging

    int    status = 0;
    Connection * dcon = _ds->getConnection(); //waits for free connect
    Routine * rtq = NULL;
    int16_t res = -1;

    
    try { //throws SQLException, connects to DB, large delay possible
        rtq = dcon->getRoutine(rtId, callStr.c_str(), true); 
    } catch (...) {
        status = -1; 
    }

    {
        MutexGuard tmp(mutex);
        if (isStopping || !_owner->hasListeners(abonent))
            rtq = NULL; //query was cancelled by either QueryManager or ThreadPool
    }

    if (rtq) {
        try {
            int timerId = -1;
            rtq->setString(rtKey, abonent.getSignals());
            if (timeOut)
                timerId = _ds->startTimer(dcon, timeOut);
            rtq->execute(); //executes query, large delay possible
            if (timerId >= 0)
                _ds->stopTimer(timerId);
            res = rtq->getInt16("RETURN"); //FUNCTION_RETURN_ATTR_NAME
        } catch (Exception& exc) {
            status = -2; 
        }
    }
    //convert returned integer to AbonentBillType value
    if (res > 0) 
        abType = smsc::inman::cache::btPrepaid;
    else if (!res)
        abType = smsc::inman::cache::btPostpaid;
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
    : _cfg(*in_cfg), _lastQId(0)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.abprov.db");
    pool.setMaxThreads((int)_cfg.max_queries);
    if (_cfg.init_threads)
        pool.preCreateThreads((int)_cfg.init_threads);
}

DBAbonentProvider::~DBAbonentProvider()
{ 
    cache = NULL;
    cancelAllQueries();
    pool.shutdown(); //waits or kills threads
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

    char*       ab_number = NULL; //AbonentId.value !!!
    CachedQuery *   ab_rec = NULL;

    QueriesHash::Iterator cit = qryCache.getIterator();
    while (cit.Next(ab_number, ab_rec)) {
        smsc_log_debug(logger, "AbProvider: cancelling query[%u:%lu](%s):%u",
                        ab_rec->qryDb->getId(), ab_rec->qryDb->Usage(), 
                       ab_number, ab_rec->cbList.size());
        ab_rec->cbList.clear();
        ab_rec->qryDb->stop();
    }
    return;
}


bool DBAbonentProvider::hasListeners(AbonentId & ab_number)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number.getSignals());
    return (ab_rec && ab_rec->cbList.size()) ? true : false;
}

//This one is called from ThreadedTask on DB query completion.
//Notifies query listeners and releases query.
void DBAbonentProvider::releaseQuery(AbonentQuery * query)
{
    MutexGuard guard(qrsGuard);
    AbonentId       ab_number = query->getAbonent();
    AbonentBillType ab_type = query->getAbonentType();
    CachedQuery *   ab_rec = qryCache.GetPtr(ab_number.getSignals());

    if (ab_rec) {
        if (ab_rec->cbList.size()) { //query wasn't cancelled
            if (cache) //update cache
                cache->setAbonentInfo(ab_number, ab_type);
            //notify listeners
            for (QueryCBList::iterator it = ab_rec->cbList.begin();
                                        it != ab_rec->cbList.end(); it++)
                (*it)->onAbonentQueried(ab_number, ab_type);
        } else
            smsc_log_debug(logger, "AbProvider: no listeners for query[%u:%lu](%s)",
                           query->getId(), query->Usage(), ab_number.getSignals());

        qryCache.Delete(ab_number.getSignals());
    } else
        smsc_log_warn(logger, "AbProvider: not in cache query[%u:%lu](%s)",
                       query->getId(), query->Usage(), ab_number.getSignals());

    qryPool.push_back(query);
    smsc_log_debug(logger, "AbProvider: query[%u:%lu](%s) is finished, type: %s (%u)",
                   query->getId(), query->Usage(), ab_number.getSignals(),
                   _sabBillType[ab_type], ab_type);
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
bool DBAbonentProvider::startQuery(const AbonentId & ab_number, 
                                   InAbonentQueryListenerITF * pf_cb/* = NULL*/)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number.getSignals());
    if (ab_rec) { //allocated query exists, just add callback to list
        ab_rec->cbList.push_back(pf_cb);
        smsc_log_debug(logger, "AbProvider: listener is added to query[%u:%lu](%s)",
                       ab_rec->qryDb->getId(), ab_rec->qryDb->Usage(),
                       ab_number.getSignals());
        return true;
    }

    CachedQuery  qryRec;
    if (qryPool.size()) {
        qryRec.qryDb = *(qryPool.begin());
        qryPool.pop_front();
    } else {
        _lastQId++;
        qryRec.qryDb = new AbonentQuery(_lastQId, this, _cfg.ds, _cfg.rtId, _cfg.rtKey);
        if (!qryRec.qryDb)
            return false;
    }
    qryRec.cbList.push_back(pf_cb);
    qryRec.qryDb->init(ab_number, _cfg.timeout);
    qryCache.Insert(ab_number.getSignals(), qryRec);
    pool.startTask(qryRec.qryDb, false); //do not delete task on completion !!!
    smsc_log_debug(logger, "AbProvider: query[%u:%lu](%s) is added to queue",
                   qryRec.qryDb->getId(), qryRec.qryDb->Usage(),
                   ab_number.getSignals());
    return true;
}

//Unbinds query listener and cancels query
void DBAbonentProvider::cancelQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb)
{
    MutexGuard  guard(qrsGuard);
    CachedQuery * ab_rec = qryCache.GetPtr(ab_number.getSignals());
    if (!ab_rec) {
        smsc_log_warn(logger, "AbProvider: attempt to cancel unexisting query!");
        return;
    }
    smsc_log_debug(logger, "AbProvider: cancelling query[%u:%lu](%s)",
                    ab_rec->qryDb->getId(), ab_rec->qryDb->Usage(),
                    ab_number.getSignals());

    QueryCBList::iterator it = std::find(ab_rec->cbList.begin(),
                                         ab_rec->cbList.end(), pf_cb);
    if (it != ab_rec->cbList.end())
        ab_rec->cbList.erase(it);
    if (ab_rec->cbList.size())
        smsc_log_debug(logger, "AbProvider: %u listeners remain for query[%u:%lu](%s)",
                       ab_rec->cbList.size(), ab_rec->qryDb->getId(),
                       ab_rec->qryDb->Usage(), ab_number.getSignals());
    return;
}

} //db
} //abprov
} //inman
} //smsc

