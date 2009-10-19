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
using smsc::util::config::ConfigView;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace db { 

class DSQryParms { //DataSource SQL Query params
public:
    struct RtParam { //sql routine params
        const char* prmId;
        char*       prmVal;
    };

    RtParam  qryParm[3];

    DSQryParms()
    {
        qryParm[0].prmId = "type";
        qryParm[1].prmId = "abonentQueryFunc";
        qryParm[2].prmId = "queryFuncArg";
    }
    ~DSQryParms()
    {
        for (int i = 0; i < 3; ++i) {
            if (qryParm[i].prmVal)
                delete [] qryParm[i].prmVal;
        }
    }

    inline const char * qryIdentVal(void) const { return qryParm[0].prmVal; }
    inline RtParam & operator[](int idx) { return qryParm[idx]; }
};

//This is the DB Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(XConfigView* use_cfg, Logger * use_log) throw(ConfigException)
{
    ConfigView dbCfg(use_cfg->relConfig(), use_cfg->relSection());

    //load drivers first, subsection: DataSourceDrivers
    if (!dbCfg.findSubSection("DataSourceDrivers"))
        throw ConfigException("'DataSourceDrivers' subsection is missed");
    try { DataSourceLoader::loadup(&dbCfg);
    } catch (const Exception & exc) {  //ConfigException or LoadupException
        throw ConfigException(exc.what());
    }

    //read DB connection parameters
    if (!dbCfg.findSubSection("DataSource"))
        throw ConfigException("'DataSource' subsection is missed");
    std::auto_ptr<ConfigView> dsCfg(dbCfg.getSubConfig("DataSource"));

    //read SQL query function parameters
    DSQryParms  _dsParm;
    for (int i = 0; i < 3; i++) {
        if (!(_dsParm[i].prmVal = dsCfg->getString(_dsParm[i].prmId)))  //allocates char[]
            throw ConfigException("'DataSource.%s' isn't set!", _dsParm[i].prmId);
    }

    DBSourceCFG  dbProvPrm;
    dbProvPrm.qryCfg.sqlRtId = _dsParm[1].prmVal;
    dbProvPrm.qryCfg.sqlRtKey = _dsParm[2].prmVal;

    dbProvPrm.qryCfg.ds = DataSourceFactory::getDataSource(_dsParm.qryIdentVal());
    if (!dbProvPrm.qryCfg.ds)
        throw ConfigException("'%s' 'DataSource' isn't registered!", _dsParm.qryIdentVal());
    dbProvPrm.qryCfg.ds->init(dsCfg.get()); //throws

    dbProvPrm.max_queries = (unsigned)dsCfg->getInt("maxQueries"); //throws
    dbProvPrm.init_threads = 1;
    bool wdog = false;
    try { wdog = dsCfg->getBool("watchdog"); }
    catch (const ConfigException & exc) { }

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
    std::string ctgr(use_log ? use_log->getName() : "smsc.inman");
    ctgr += ".iap.db";
    logger = Logger::getInstance(ctgr.c_str());
    qryCfg = use_cfg.qryCfg;
    prvdCfg.init_threads = use_cfg.init_threads;
    prvdCfg.max_queries = use_cfg.max_queries;
    prvdCfg.qryMultiRun = true; //The prepared SQL Routine is reused !!!
    prvdCfg.qryPlant = new DBQueryFactory(qryCfg, qryCfg.timeOut_secs, logger);
}

IAProviderCreatorDB::~IAProviderCreatorDB()
{
    if (qryCfg.ds)
        delete qryCfg.ds;
    if (prvdCfg.qryPlant)
        delete prvdCfg.qryPlant;
}

IAPQueryProcessorITF * IAProviderCreatorDB::startProvider(const ICServicesHostITF * use_host)
{
    MutexGuard grd(_sync);
    if (!prvd.get()) 
        prvd.reset(new IAPQueryFacility(prvdCfg, logger));
    return prvd.get();
}

void IAProviderCreatorDB::stopProvider(bool do_wait/* = false*/)
{
    MutexGuard grd(_sync);
    if (prvd.get())
        prvd->Stop(do_wait);
}

void  IAProviderCreatorDB::logConfig(Logger * use_log/* = NULL*/) const
{
    if (!use_log)
        use_log = logger;
    smsc_log_info(use_log, "FUNCTION %s(%s IN VARCHAR)", qryCfg.rtId(), qryCfg.rtKey());
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
    callStr += _cfg.rtId(); callStr += "(:";
    callStr += _cfg.rtKey(); callStr += ");";
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
        rtq = dcon->getRoutine(_cfg.rtId(), callStr.c_str(), true); 
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
            rtq->setString(_cfg.rtKey(), abonent.getSignals());
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

