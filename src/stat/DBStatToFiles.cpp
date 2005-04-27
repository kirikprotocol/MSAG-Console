
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

#include <core/threads/ThreadPool.hpp>

#include "StatisticsManager.h"

using namespace smsc::db;
using namespace smsc::stat;
using namespace smsc::core::threads;
    
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

smsc::logger::Logger* logger = 0;

const char* GENERAL_STAT_SQL = 
"SELECT sms_stat_sms.period, "
"       sms_stat_state.errcode, sum(sms_stat_state.counter), "
"       sum(sms_stat_sms.accepted), sum(sms_stat_sms.rejected), "
"       sum(sms_stat_sms.delivered), sum(sms_stat_sms.failed), "
"       sum(sms_stat_sms.rescheduled), sum(sms_stat_sms.temporal), "
"       max(sms_stat_sms.peak_i), max(sms_stat_sms.peak_o) "
"FROM   sms_stat_sms, sms_stat_state "
"WHERE  sms_stat_state.period = sms_stat_sms.period "
"GROUP BY sms_stat_sms.period, sms_stat_state.errcode "
"ORDER BY sms_stat_sms.period ASC";

const char* SME_STAT_ID  = "SME_STAT_ID"; 
const char* SME_STAT_SQL = 
"SELECT sms_stat_sme.systemid, "
"       sms_stat_sme_state.errcode, sum(sms_stat_sme_state.counter), "
"       sum(sms_stat_sme.accepted), sum(sms_stat_sme.rejected), "
"       sum(sms_stat_sme.delivered), sum(sms_stat_sme.failed), "   
"       sum(sms_stat_sme.rescheduled), sum(sms_stat_sme.temporal), "
"       max(sms_stat_sme.peak_i), max(sms_stat_sme.peak_o) "
"FROM   sms_stat_sme, sms_stat_sme_state "
"WHERE  sms_stat_sme.systemid = sms_stat_sme_state.systemid "
"  AND  sms_stat_sme.period = :period "
"GROUP BY sms_stat_sme.systemid, sms_stat_sme_state.errcode "
"ORDER BY sms_stat_sme.systemid ASC";

const char* ROUTE_STAT_ID  = "ROUTE_STAT_ID";
const char* ROUTE_STAT_SQL = 
"SELECT sms_stat_route.routeid, "
"       sms_stat_route_state.errcode, sum(sms_stat_route_state.counter), "
"       sum(sms_stat_route.accepted), sum(sms_stat_route.rejected), "
"       sum(sms_stat_route.delivered), sum(sms_stat_route.failed), "
"       sum(sms_stat_route.rescheduled), sum(sms_stat_route.temporal), "
"       max(sms_stat_route.peak_i), max(sms_stat_route.peak_o) "
"FROM  sms_stat_route, sms_stat_route_state "
"WHERE sms_stat_route.routeid = sms_stat_route_state.routeid "
"  AND sms_stat_route.period = :period "
"GROUP BY sms_stat_route.routeid, sms_stat_route_state.errcode "
"ORDER BY sms_stat_route.routeid ASC";

const char* ERR_STMT_MESSAGE = "Failed to obtain %s statement";
const char* ERR_RS_MESSAGE   = "Failed to get %s result set";

void convertPeriod(uint32_t period, tm& flushTM)
{
    //period format = YYYYMMDDHH (by local time)
    flushTM.tm_year = (period/1000000) - 1900;
    flushTM.tm_mon  = (period%1000000)/10000 - 1;
    flushTM.tm_mday = (period%10000)/100;
    flushTM.tm_hour = (period%100);
    flushTM.tm_min  = 0; flushTM.tm_sec = 0;
    time_t flushTime = mktime(&flushTM);
    gmtime_r(&flushTime, &flushTM); flushTM.tm_sec = 0;
}

void fillSmes(Connection* connection, Hash<SmsStat>& stat, uint32_t period)
{
    Statement* stmt = connection->getStatement(SME_STAT_ID, SME_STAT_SQL);
    if (!stmt) throw Exception(ERR_STMT_MESSAGE, "smes");
    stmt->setUint32(1, period);
    std::auto_ptr<ResultSet> rsGuard(stmt->executeQuery());
    ResultSet* rs = rsGuard.get();
    if (!rs) throw Exception(ERR_RS_MESSAGE, "smes");

    while (rs->fetchNext())
    {
        const char* smeId = (rs->isNull(1) ? 0:rs->getString(1));
        if (!smeId || !smeId[0]) { smsc_log_warn(logger, "Got null sme_id"); continue; }
        
        SmsStat* sstat = stat.GetPtr(smeId);
        if (!sstat) // new route record starts => insert new
        {
            SmsStat newStat; // empty
            stat.Insert(smeId, newStat);
            sstat = stat.GetPtr(smeId);
        }

        if (!rs->isNull(2)) // increment errors statistics
        { 
            int32_t errcode = rs->getInt32(2);
            int32_t counter = (rs->isNull(3) ? 0:rs->getInt32(3));
            StatisticsManager::addError(sstat->errors, errcode, counter);
        }

        sstat->accepted    += (rs->isNull (4) ? 0:rs->getInt32 (4));
        sstat->rejected    += (rs->isNull (5) ? 0:rs->getInt32 (5));
        sstat->delivered   += (rs->isNull (6) ? 0:rs->getInt32 (6));
        sstat->failed      += (rs->isNull (7) ? 0:rs->getInt32 (7));
        sstat->rescheduled += (rs->isNull (8) ? 0:rs->getInt32 (8));
        sstat->temporal    += (rs->isNull (9) ? 0:rs->getInt32 (9));
        int32_t peak_i      = (rs->isNull(10) ? 0:rs->getInt32(10));
        int32_t peak_o      = (rs->isNull(11) ? 0:rs->getInt32(11));
        if (sstat->peak_i < peak_i) sstat->peak_i = peak_i;
        if (sstat->peak_i < peak_o) sstat->peak_o = peak_o;
    }
}
void fillRoutes(Connection* connection, Hash<RouteStat>& stat, uint32_t period)
{
    Statement* stmt = connection->getStatement(ROUTE_STAT_ID, ROUTE_STAT_SQL);
    if (!stmt) throw Exception(ERR_STMT_MESSAGE, "routes");
    stmt->setUint32(1, period);
    std::auto_ptr<ResultSet> rsGuard(stmt->executeQuery());
    ResultSet* rs = rsGuard.get();
    if (!rs) throw Exception(ERR_RS_MESSAGE, "routes");

    while (rs->fetchNext())
    {
        const char* routeId = (rs->isNull(1) ? 0:rs->getString(1));
        if (!routeId || !routeId[0]) { smsc_log_warn(logger, "Got null route_id"); continue; }
        
        RouteStat* rstat = stat.GetPtr(routeId);
        if (!rstat) // new route record starts => insert new
        {
            RouteStat newStat; // empty & (providerId == categoryId == -1)
            stat.Insert(routeId, newStat);
            rstat = stat.GetPtr(routeId);
        }

        if (!rs->isNull(2)) // increment errors statistics
        { 
            int32_t errcode = rs->getInt32(2);
            int32_t counter = (rs->isNull(3) ? 0:rs->getInt32(3));
            StatisticsManager::addError(rstat->errors, errcode, counter);
        }

        rstat->accepted    += (rs->isNull (4) ? 0:rs->getInt32 (4));
        rstat->rejected    += (rs->isNull (5) ? 0:rs->getInt32 (5));
        rstat->delivered   += (rs->isNull (6) ? 0:rs->getInt32 (6));
        rstat->failed      += (rs->isNull (7) ? 0:rs->getInt32 (7));
        rstat->rescheduled += (rs->isNull (8) ? 0:rs->getInt32 (8));
        rstat->temporal    += (rs->isNull (9) ? 0:rs->getInt32 (9));
        int32_t peak_i      = (rs->isNull(10) ? 0:rs->getInt32(10));
        int32_t peak_o      = (rs->isNull(11) ? 0:rs->getInt32(11));
        if (rstat->peak_i < peak_i) rstat->peak_i = peak_i;
        if (rstat->peak_i < peak_o) rstat->peak_o = peak_o;
    }
}
void process(Connection* connection, const char* location)
{
    smsc_log_info(logger, "Dump process started ...");
    
    std::auto_ptr<Statement> generalGuard(connection->createStatement(GENERAL_STAT_SQL));
    Statement* general = generalGuard.get();
    if (!general) throw Exception(ERR_STMT_MESSAGE, "general");
    std::auto_ptr<ResultSet> genRsGuard(general->executeQuery());
    ResultSet* genRs = genRsGuard.get();
    if (!genRs) throw Exception(ERR_RS_MESSAGE, "general");
    
    StatStorage storage(location); tm flushTM;
    
    SmsStat         stat;
    Hash<SmsStat>   statSme;
    Hash<RouteStat> statRoute;
    
    uint32_t period  = 0; 
    uint32_t lastPeriod = 0;

    while (genRs->fetchNext())
    {
        period = (genRs->isNull(1) ? 0:genRs->getUint32(1));
        if (period == 0) { smsc_log_warn(logger, "Got null period"); continue; }
        
        if (lastPeriod != 0 && lastPeriod != period) // if new period starts (all errors was got)
        {
            fillSmes(connection, statSme, lastPeriod);
            fillRoutes(connection, statRoute, lastPeriod);
            
            StatisticsManager::flush(flushTM, storage, stat, statSme, statRoute);
            stat.Empty(); statSme.Empty(); statRoute.Empty();
            convertPeriod(period, flushTM); // sets flushTM structure
        }
        
        if (!genRs->isNull(2)) // increment general errors statistics
        { 
            int32_t errcode = genRs->getInt32(2);
            int32_t counter = genRs->isNull(3) ? 0:genRs->getInt32(3);
            StatisticsManager::addError(stat.errors, errcode, counter);
        }

        stat.accepted    += (genRs->isNull (4) ? 0:genRs->getInt32 (4));
        stat.rejected    += (genRs->isNull (5) ? 0:genRs->getInt32 (5));
        stat.delivered   += (genRs->isNull (6) ? 0:genRs->getInt32 (6));
        stat.failed      += (genRs->isNull (7) ? 0:genRs->getInt32 (7));
        stat.rescheduled += (genRs->isNull (8) ? 0:genRs->getInt32 (8));
        stat.temporal    += (genRs->isNull (9) ? 0:genRs->getInt32 (9));
        int32_t peak_i    = (genRs->isNull(10) ? 0:genRs->getInt32(10));
        int32_t peak_o    = (genRs->isNull(11) ? 0:genRs->getInt32(11));
        if (stat.peak_i < peak_i) stat.peak_i = peak_i;
        if (stat.peak_i < peak_o) stat.peak_o = peak_o;
        lastPeriod = period;
    }

    if (period != 0) // process & flush data for last period
    {
        convertPeriod(period, flushTM);
        fillSmes(connection, statSme, period);
        fillRoutes(connection, statRoute, period);
        StatisticsManager::flush(flushTM, storage, stat, statSme, statRoute);
    }
    
    smsc_log_info(logger, "Dump process finished");
}

int main(void)
{
    Logger::Init();
    logger = Logger::getInstance("smsc.stat.DBStatToFiles");
    
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    DataSourceLoader::loadupDataSourceFactory("../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);
    DataSource* ds = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    
    if (!ds) {
        smsc_log_error(logger, "Create DataSource failed!");
        return -1;
    }
    
    Manager::init("config.xml");
    std::auto_ptr<ConfigView> dsConfig(new ConfigView(Manager::getInstance(), "DataSource"));
    ds->init(dsConfig.get());
    std::auto_ptr<ConfigView> msConfig(new ConfigView(Manager::getInstance(), "MessageStore"));
    const char* location = msConfig.get()->getString("statisticsDir");
    
    Connection* connection = 0;
    try 
    {
        connection = ds->getConnection();
        if (!connection) 
            throw Exception("Failed to obtain connection to DB");
        
        process(connection, location);
        
        ds->freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (connection) ds->freeConnection(connection);
        smsc_log_error(logger, "Exception occured: %s", exc.what());
    }
    
    smsc_log_info(logger, "Exit");
    return 0;
}
