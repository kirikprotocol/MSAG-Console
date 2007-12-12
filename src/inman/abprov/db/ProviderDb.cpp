#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/abprov/IAPLoader.hpp"
#include "inman/abprov/db/ProviderDb.hpp"

using smsc::db::Connection;
using smsc::db::Routine;

#include "db/DataSourceLoader.h"
using smsc::db::DataSourceLoader;
using smsc::db::DataSourceFactory;

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
namespace iaprvd {
namespace db { 

//This is the DB Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
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
    dbProvPrm.qryCfg.rtId = _dsParm[1].prmVal;
    dbProvPrm.qryCfg.rtKey = _dsParm[2].prmVal;

    dbProvPrm.qryCfg.ds = DataSourceFactory::getDataSource(_DS_IDENT_VAL);
    if (!dbProvPrm.qryCfg.ds)
        throw ConfigException("'%s' 'DataSource' isn't registered!", _DS_IDENT_VAL);
    dbProvPrm.qryCfg.ds->init(dsCfg); //throws

    dbProvPrm.max_queries = (unsigned)dsCfg->getInt("maxQueries"); //throws
    dbProvPrm.init_threads = 1;
    bool wdog = false;
    try { wdog = dsCfg->getBool("watchdog"); }
    catch (ConfigException & exc) { }

    if (wdog)
        dbProvPrm.qryCfg.timeOut_secs = (unsigned)dsCfg->getInt("timeout"); //throws
    return new IAProviderCreatorDB(dbProvPrm, use_log);
}

/* ************************************************************************** *
 * class IAProviderCreatorDB implementation:
 * ************************************************************************** */
IAProviderCreatorDB::IAProviderCreatorDB(const DBSourceCFG & use_cfg,
                                         Logger * use_log/* = NULL*/)
{
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.db");
    qryCfg = use_cfg.qryCfg;
    prvdCfg.init_threads = use_cfg.init_threads;
    prvdCfg.max_queries = use_cfg.max_queries;
    prvdCfg.qryMultiRun = true; //The prepared SQL Routine is reused !!!
    prvdCfg.qryPlant = new DBQueryFactory(qryCfg, qryCfg.timeOut_secs, logger);
}

IAProviderCreatorDB::~IAProviderCreatorDB()
{
    ProvidersLIST::iterator it = prvdList.begin();
    for (; it != prvdList.end(); it++)
        delete (*it);
    prvdList.clear();

    if (qryCfg.ds)
        delete qryCfg.ds;
    if (prvdCfg.qryPlant)
        delete prvdCfg.qryPlant;
}

IAProviderITF * IAProviderCreatorDB::create(Logger * use_log)
{
    IAProviderThreaded * prov = new IAProviderThreaded(prvdCfg, use_log);
    prvdList.push_back(prov);
    return prov;
}

void  IAProviderCreatorDB::logConfig(Logger * use_log) const
{
    smsc_log_info(use_log, "FUNCTION %s(%s IN VARCHAR)", qryCfg.rtId, qryCfg.rtKey);
    smsc_log_info(use_log, "Max.queries: %u", prvdCfg.max_queries);
    smsc_log_info(use_log, "Query timeout: %u secs", qryCfg.timeOut_secs);
}

/* ************************************************************************** *
 * class DBQueryFactory implementation:
 * ************************************************************************** */
DBQueryFactory::DBQueryFactory(const IAPQueryDB_CFG &in_cfg, unsigned timeout_secs,
                               Logger * use_log/* = NULL*/)
    : _cfg(in_cfg)
{
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.db");
}

IAPQueryAC * DBQueryFactory::newQuery(unsigned q_id, IAPQueryManagerITF * owner,
                                         Logger * use_log)
{
    return new IAPQueryDB(q_id, owner, use_log, _cfg);
}

/* ************************************************************************** *
 * class IAPQueryDB implementation:
 * ************************************************************************** */
IAPQueryDB::IAPQueryDB(unsigned q_id, IAPQueryManagerITF * owner, 
                        Logger * use_log, const IAPQueryDB_CFG & use_cfg)
    : IAPQueryAC(q_id, owner, use_cfg.timeOut_secs, use_log), _cfg(use_cfg)
{
    callStr += _cfg.rtId; callStr += "(:";
    callStr += _cfg.rtKey; callStr += ");";
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.db");
    mkTaskName();
}

int IAPQueryDB::Execute(void)
{ 
    {
        MutexGuard tmp(_mutex);
        if (isStopping || !_owner->hasListeners(abonent)) {
            //query was cancelled by either QueryManager or ThreadPool
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqCancelled);
            return _qStatus;
        }
    }
    //sleep(24); //for debugging

    Connection * dcon = _cfg.ds->getConnection(); //waits for free connect
    Routine * rtq = NULL;
    int16_t res = -1;

    
    try { //throws SQLException, connects to DB, large delay possible
        rtq = dcon->getRoutine(_cfg.rtId, callStr.c_str(), true); 
    } catch (const std::exception& exc) {
        smsc_log_error(logger, "%s(%s): %s", taskName(),
                       abonent.getSignals(), exc.what());
        _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqError);
        _exc = exc.what();
    }

    {
        MutexGuard tmp(_mutex);
        if (isStopping || !_owner->hasListeners(abonent)) {
            //query was cancelled by either QueryManager or ThreadPool
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqCancelled);
            rtq = NULL;
        }
    }

    if (rtq) {
        try {
            int timerId = -1;
            rtq->setString(_cfg.rtKey, abonent.getSignals());
            if (timeOut)
                timerId = _cfg.ds->startTimer(dcon, timeOut);
            rtq->execute(); //executes query, large delay possible
            if (timerId >= 0)
                _cfg.ds->stopTimer(timerId);
            res = rtq->getInt16("RETURN"); //FUNCTION_RETURN_ATTR_NAME
        } catch (const std::exception& exc) {
            smsc_log_error(logger, "%s(%s): %s", taskName(), 
                           abonent.getSignals(), exc.what());
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqError);
            _exc = exc.what();
        }
    }
    //convert returned integer to AbonentContractType value
    if (res > 0) 
        abInfo.abRec.ab_type = AbonentContractInfo::abtPrepaid;
    else if (!res)
        abInfo.abRec.ab_type = AbonentContractInfo::abtPostpaid;
    //else abtUnknown

    //do not unregister routine in order to reuse it in next queries.
    //routine will be deleted by ~DataSource().
    _cfg.ds->freeConnection(dcon);
    return _qStatus;
}

} //db
} //iaprvd
} //inman
} //smsc

