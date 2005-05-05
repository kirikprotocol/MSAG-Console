
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
"SELECT period, " 
"       sum(accepted), sum(rejected), sum(delivered), sum(failed), "
"       sum(rescheduled), sum(temporal), max(peak_i), max(peak_o) "
"FROM sms_stat_sms GROUP BY period ORDER BY period ASC";
const char* GENERAL_STAT_ERR_SQL = 
"SELECT errcode, sum(counter) "
"FROM sms_stat_state WHERE period = :period "
"GROUP BY errcode ORDER BY errcode ASC";

const char* SME_STAT_SQL = 
"SELECT systemid, " 
"       sum(accepted), sum(rejected), sum(delivered), sum(failed), "   
"       sum(rescheduled), sum(temporal), max(peak_i), max(peak_o) "
"FROM sms_stat_sme WHERE period = :period "
"GROUP BY systemid ORDER BY systemid ASC";
const char* SME_STAT_ERR_SQL = 
"SELECT systemid, errcode, sum(counter) "
"FROM sms_stat_sme_state WHERE period = :period "
"GROUP BY systemid, errcode ORDER BY systemid ASC, errcode ASC";

const char* ROUTE_STAT_SQL = 
"SELECT routeid, "
"       sum(accepted), sum(rejected), sum(delivered), sum(failed), "   
"       sum(rescheduled), sum(temporal), max(peak_i), max(peak_o) "
"FROM  sms_stat_route WHERE period = :period "
"GROUP BY routeid ORDER BY routeid ASC";
const char* ROUTE_STAT_ERR_SQL = 
"SELECT routeid, errcode, sum(counter) "
"FROM sms_stat_route_state WHERE period = :period "
"GROUP BY routeid, errcode ORDER BY routeid ASC, errcode ASC";

const char* ERR_STMT_MESSAGE = "Failed to obtain %s statement";
const char* ERR_RS_MESSAGE   = "Failed to get %s result set";

void convertPeriod(uint32_t period, tm& flushTM)
{
    //period format = YYYYMMDDHH (by local time)
    flushTM.tm_isdst = -1;
    flushTM.tm_year = (period/1000000) - 1900;
    flushTM.tm_mon  = (period%1000000)/10000 - 1;
    flushTM.tm_mday = (period%10000)/100;
    flushTM.tm_hour = (period%100);
    flushTM.tm_min  = 0; flushTM.tm_sec = 0;
    time_t flushTime = mktime(&flushTM);
    gmtime_r(&flushTime, &flushTM);
}

void incError(SmsStat* stat, ResultSet* rs, int pos)
{
    if (!rs->isNull(pos)) // increment errors statistics
    { 
        int32_t errcode = rs->getInt32(pos); pos++;
        int32_t counter = (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
        StatisticsManager::addError(stat->errors, errcode, counter);
    }
}
void incCounters(SmsStat* stat, ResultSet* rs, int pos)
{
    stat->accepted    += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    stat->rejected    += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    stat->delivered   += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    stat->failed      += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    stat->rescheduled += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    stat->temporal    += (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    int32_t peak_i     = (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    int32_t peak_o     = (rs->isNull(pos) ? 0:rs->getInt32(pos)); pos++;
    if (stat->peak_i < peak_i) stat->peak_i = peak_i;
    if (stat->peak_i < peak_o) stat->peak_o = peak_o;
}

void fillSmes(Hash<SmsStat>& stat, ResultSet* rs, ResultSet* errRs)
{
    while (rs && rs->fetchNext())
    {
        const char* smeId = (rs->isNull(1) ? 0:rs->getString(1));
        if (!smeId || !smeId[0]) { smsc_log_warn(logger, "Got null sme_id"); continue; }
        
        SmsStat* sstat = stat.GetPtr(smeId);
        if (!sstat) // new route sme starts => insert new
        {
            SmsStat newStat; // empty
            stat.Insert(smeId, newStat);
            sstat = stat.GetPtr(smeId);
        }
        incCounters(sstat, rs, 2);
    }
    while (errRs && errRs->fetchNext())
    {
        const char* smeId = (rs->isNull(1) ? 0:rs->getString(1));
        if (!smeId || !smeId[0]) { smsc_log_warn(logger, "Got null err sme_id"); continue; }

        SmsStat* sstat = stat.GetPtr(smeId);
        if (!sstat) { smsc_log_warn(logger, "Sme '%s' record not found", smeId); continue; }
        incError(sstat, errRs, 2);
    }
}
void fillRoutes(Hash<RouteStat>& stat, ResultSet* rs, ResultSet* errRs)
{
    while (rs && rs->fetchNext())
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
        incCounters(rstat, rs, 2);
    }
    while (errRs && errRs->fetchNext())
    {
        const char* routeId = (rs->isNull(1) ? 0:rs->getString(1));
        if (!routeId || !routeId[0]) { smsc_log_warn(logger, "Got null route_id"); continue; }
        
        RouteStat* rstat = stat.GetPtr(routeId);
        if (!rstat) { smsc_log_warn(logger, "Route '%s' record not found", routeId); continue; }
        incError(rstat, errRs, 2);
    }
}
void process(Connection* connection, const char* location)
{
    smsc_log_info(logger, "Dump process started, creating statements...");
    
    std::auto_ptr<Statement> generalGuard(connection->createStatement(GENERAL_STAT_SQL));
    Statement* general = generalGuard.get();
    if (!general) throw Exception(ERR_STMT_MESSAGE, "general");
    std::auto_ptr<ResultSet> genRsGuard(general->executeQuery());
    ResultSet* genRs = genRsGuard.get();
    if (!genRs) throw Exception(ERR_RS_MESSAGE, "general");
    std::auto_ptr<Statement> generalErrGuard(connection->createStatement(GENERAL_STAT_ERR_SQL));
    Statement* generalErr = generalErrGuard.get();
    if (!generalErr) throw Exception(ERR_STMT_MESSAGE, "general err");

    std::auto_ptr<Statement> smeGuard(connection->createStatement(SME_STAT_SQL));
    Statement* sme = smeGuard.get();
    if (!sme) throw Exception(ERR_STMT_MESSAGE, "sme");
    std::auto_ptr<Statement> smeErrGuard(connection->createStatement(SME_STAT_ERR_SQL));
    Statement* smeErr = smeErrGuard.get();
    if (!sme) throw Exception(ERR_STMT_MESSAGE, "sme err");

    std::auto_ptr<Statement> routeGuard(connection->createStatement(ROUTE_STAT_SQL));
    Statement* route = routeGuard.get();
    if (!route) throw Exception(ERR_STMT_MESSAGE, "route");
    std::auto_ptr<Statement> routeErrGuard(connection->createStatement(ROUTE_STAT_ERR_SQL));
    Statement* routeErr = routeErrGuard.get();
    if (!routeErr) throw Exception(ERR_STMT_MESSAGE, "route err");

    StatStorage storage(location); tm flushTM;
    
    SmsStat         stat;
    Hash<SmsStat>   statSme;
    Hash<RouteStat> statRoute;

    smsc_log_info(logger, "Statements creating, fecthing records...");

    while (genRs->fetchNext())
    {
        uint32_t period = (genRs->isNull(1) ? 0:genRs->getUint32(1));
        if (period == 0) { smsc_log_warn(logger, "Got null period"); continue; }

        incCounters(&stat, genRs, 2);

        {   // get & add all errors stat
            generalErr->setUint32(1, period);
            std::auto_ptr<ResultSet> genErrRsGuard(generalErr->executeQuery());
            ResultSet* genErrRs = genErrRsGuard.get();
            while (genErrRs && genErrRs->fetchNext()) incError(&stat, genErrRs, 1);
        }
        {
            sme->setUint32(1, period);
            smeErr->setUint32(1, period);
            std::auto_ptr<ResultSet> smeRsGuard(sme->executeQuery());
            std::auto_ptr<ResultSet> smeErrRsGuard(smeErr->executeQuery());
            fillSmes(statSme, smeRsGuard.get(), smeErrRsGuard.get());
        }
        {
            route->setUint32(1, period);
            routeErr->setUint32(1, period);
            std::auto_ptr<ResultSet> routeRsGuard(route->executeQuery());
            std::auto_ptr<ResultSet> routeErrRsGuard(routeErr->executeQuery());
            fillRoutes(statRoute, routeRsGuard.get(), routeErrRsGuard.get());
        }
        
        convertPeriod(period, flushTM);
        smsc_log_debug(logger, "Dumping period %ld", period);
        StatisticsManager::flush(flushTM, storage, stat, statSme, statRoute);
        stat.Empty(); statSme.Empty(); statRoute.Empty();
    }

    smsc_log_info(logger, "Dump process finished");
}

int main(void)
{
    Logger::Init();
    logger = Logger::getInstance("smsc.stat.DBStatToFiles");
    
    Manager::init("config.xml");
    ConfigView dsLoaderConfig(Manager::getInstance(), "StartupLoader");
    DataSourceLoader::loadup(&dsLoaderConfig);
    
    std::auto_ptr<ConfigView> dsCfgGuard(new ConfigView(Manager::getInstance(), "DataSource"));
    ConfigView* dsConfig = dsCfgGuard.get();
    std::auto_ptr<char> dsIdentity(dsConfig->getString("type"));
    const char* dsIdentityStr = dsIdentity.get();
    DataSource* ds = DataSourceFactory::getDataSource(dsIdentityStr);
    if (!ds) {
        smsc_log_error(logger, "Create DataSource failed!");
        return -1;
    }
    ds->init(dsConfig);
    
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
    
    DataSourceLoader::unload();
    smsc_log_info(logger, "Exit");
    return 0;
}
