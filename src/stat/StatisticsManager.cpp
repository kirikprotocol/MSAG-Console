
#include "StatisticsManager.h"

namespace smsc { namespace stat
{

StatisticsManager::StatisticsManager(DataSource& _ds)
    : Statistics(), ThreadedTask(),
        logger(Logger::getInstance("smsc.stat.StatisticsManager")),
            ds(_ds), currentIndex(0), isStarted(false), bExternalFlush(false)
{
    resetCounters(0); resetCounters(1);
}
StatisticsManager::~StatisticsManager()
{
    MutexGuard guard(stopLock);
}

void StatisticsManager::addError(IntHash<int>& hash, int errcode)
{
    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, 1);
    else (*counter)++;
}

// SMS accepted by SMSC, affects accepted only
void StatisticsManager::updateAccepted(const char* srcSmeId, const char* routeId)
{
    MutexGuard  switchGuard(switchLock);
    
    statGeneral[currentIndex].accepted++; 
    statGeneral[currentIndex].incICounter();
    
    if (srcSmeId && srcSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        if (stat) stat->accepted++;
        else {
            statBySmeId[currentIndex].Insert(srcSmeId, SmsStat(1));
            stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        }
        if (stat) stat->incICounter();
    }
    if (routeId && routeId[0])
    {
        SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) stat->accepted++;
        else {
            statByRoute[currentIndex].Insert(routeId, SmsStat(1));
            stat = statByRoute[currentIndex].GetPtr(routeId);
        }
        if (stat) stat->incICounter();
    }
}
// SMS rejected by SMSC. Affects rejected && errors only
void StatisticsManager::updateRejected(const char* srcSmeId, const char* routeId, int errcode)
{
    MutexGuard  switchGuard(switchLock);

    if (errcode != 0)
    {
        statGeneral[currentIndex].rejected++;

        if (srcSmeId && srcSmeId[0])
        {
            SmsStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
            if (stat) {
                stat->rejected++;
                addError(stat->errors, errcode);
            }
            else {
                SmsStat newStat(0, 1); // rejected
                addError(newStat.errors, errcode);
                statBySmeId[currentIndex].Insert(srcSmeId, newStat);
            }
        }

        if (routeId && routeId[0])
        {
            SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->rejected++;
                addError(stat->errors, errcode);
            }
            else {
                SmsStat newStat(0, 1); // rejected
                addError(newStat.errors, errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    addError(statGeneral[currentIndex].errors, errcode);
}

// SMS was't delivered by SMSC with temporal error. Affects temporal && errors only 
void StatisticsManager::updateTemporal(const char* dstSmeId, const char* routeId, int errcode)
{
    MutexGuard  switchGuard(switchLock);

    if (errcode != 0)
    {
        statGeneral[currentIndex].temporal++;

        if (dstSmeId && dstSmeId[0])
        {
            SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
            if (stat) {
                stat->temporal++;
                addError(stat->errors, errcode);
            }
            else {
                SmsStat newStat(0, 0, 0, 0, 0, 1); // temporal
                addError(newStat.errors, errcode);
                statBySmeId[currentIndex].Insert(dstSmeId, newStat);
            }
        }
        
        if (routeId && routeId[0])
        {
            SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->temporal++;
                addError(stat->errors, errcode);
            }
            else {
                SmsStat newStat(0, 0, 0, 0, 0, 1); // temporal
                addError(newStat.errors, errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    addError(statGeneral[currentIndex].errors, errcode);
}

// SMS was delivered or failed by SMSC. Affects delivered or failed
void StatisticsManager::updateChanged(const char* dstSmeId, const char* routeId, int errcode)
{
    MutexGuard  switchGuard(switchLock);

    if (dstSmeId && dstSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
        if (stat) {
            if (errcode == 0) stat->delivered++;
            else stat->failed++;
            addError(stat->errors, errcode);
        }
        else {
            SmsStat newStat(0, 0, (errcode) ? 0:1, (errcode) ? 1:0, 0, 0); // delivered or failed
            addError(newStat.errors, errcode);
            statBySmeId[currentIndex].Insert(dstSmeId, newStat);
            if (errcode == 0) {
                stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
            }
        }
        if (errcode == 0 && stat) stat->incOCounter();
    }
    
    if (routeId && routeId[0])
    {
        SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) {
            if (errcode == 0) stat->delivered++;
            else stat->failed++;
            addError(stat->errors, errcode);
        }
        else {
            SmsStat newStat(0, 0, (errcode) ? 0:1, (errcode) ? 1:0, 0, 0); // delivered or failed
            addError(newStat.errors, errcode);
            statByRoute[currentIndex].Insert(routeId, newStat);
            if (errcode == 0) {
                stat = statByRoute[currentIndex].GetPtr(routeId);
            }
        }
        if (errcode == 0 && stat) stat->incOCounter();
    }
    
    if (errcode == 0) {
        statGeneral[currentIndex].delivered++;
        statGeneral[currentIndex].incOCounter();
    }
    else statGeneral[currentIndex].failed++;
    addError(statGeneral[currentIndex].errors, errcode);
}
void StatisticsManager::updateScheduled(const char* dstSmeId, const char* routeId)
{
    MutexGuard  switchGuard(switchLock);

    if (dstSmeId && dstSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
        if (stat) stat->rescheduled++;
        else statBySmeId[currentIndex].Insert(dstSmeId, SmsStat(0, 0, 0, 0, 1, 0)); 
    }
    
    if (routeId && routeId[0])
    {
        SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) stat->rescheduled++;
        else statByRoute[currentIndex].Insert(routeId, SmsStat(0, 0, 0, 0, 1, 0));
    }
    
    statGeneral[currentIndex].rescheduled++;
}

int StatisticsManager::Execute()
{
    smsc_log_debug(logger, "Execute() started (%d)", isStopping);
    isStarted = true; bExternalFlush = false;
    while (!isStopping)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "Execute() >> End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "Execute() >> Flushed");
    }
    isStarted = false;
    exitEvent.Signal();
    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void StatisticsManager::stop()
{
    MutexGuard guard(stopLock);

    smsc_log_debug(logger, "stop() called, started=%d", isStarted);
    ThreadedTask::stop();
    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
        smsc_log_debug(logger, "stop() waiting finish ...");
        exitEvent.Wait();
    }
    smsc_log_debug(logger, "stop() exited");
}

void StatisticsManager::flushStatistics()
{
    MutexGuard flushGuard(flushLock);

    if (doneEvent.isSignaled()) doneEvent.Wait(0);
    bExternalFlush = true;
    awakeEvent.Signal();
    doneEvent.Wait();
}

short StatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    short flushIndex = currentIndex;
    currentIndex = (currentIndex == 0) ? 1:0;
    return flushIndex;
}

uint32_t StatisticsManager::calculatePeriod()
{
    time_t currTime = time(0);
    if (!bExternalFlush) currTime -= 3600;
    tm tmCT; localtime_r(&currTime, &tmCT);
    return  (tmCT.tm_year+1900)*1000000+(tmCT.tm_mon+1)*10000+
            (tmCT.tm_mday)*100+tmCT.tm_hour;
}
int StatisticsManager::calculateToSleep() // returns msecs to next hour
{
    time_t currTime = time(0);
    time_t nextTime = currTime + 3600;
    tm tmNT; localtime_r(&nextTime, &tmNT);
    tmNT.tm_sec = 0; tmNT.tm_min = 0;
    nextTime = mktime(&tmNT);
    return (((nextTime-currTime)*1000)+1);
}

const char* insertStatSmsSql = (const char*)
"INSERT INTO sms_stat_sms (period, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o) "
"VALUES (:period, :accepted, :rejected, :delivered, :failed, :rescheduled, :temporal, :peak_i, :peak_o)";
const char* insertStatSmeSql = (const char*)
"INSERT INTO sms_stat_sme (period, systemid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)\
 VALUES (:period, :systemid, :accepted, :rejected, :delivered, :failed, :rescheduled, :temporal, :peak_i, :peak_o)";
const char* insertStatRouteSql = (const char*)
"INSERT INTO sms_stat_route (period, routeid, accepted, rejected, delivered, failed, rescheduled, temporal, peak_i, peak_o)\
 VALUES (:period, :routeid, :accepted, :rejected, :delivered, :failed, :rescheduled, :temporal, :peak_i, :peak_o)";
const char* insertStatStateSql = (const char*)
"INSERT INTO sms_stat_state (period, errcode, counter) "
"VALUES (:period, :errcode, :counter)";
const char* insertStatSmeStateSql = (const char*)
"INSERT INTO sms_stat_sme_state (period, systemid, errcode, counter) "
"VALUES (:period, :systemid, :errcode, :counter)";
const char* insertStatRouteStateSql = (const char*)
"INSERT INTO sms_stat_route_state (period, routeid, errcode, counter) "
"VALUES (:period, :routeid, :errcode, :counter)";

void StatisticsManager::flushCounters(short index)
{
    uint32_t period = calculatePeriod();

    smsc_log_debug(logger, "Flushing statistics for period: %d / %d", period, time(0));

    Connection* connection = 0;

    Statement* insertStatSmeStmt    = 0;
    Statement* insertStatSmsStmt    = 0;
    Statement* insertStatRouteStmt  = 0;
    Statement* insertStatStateStmt       = 0;
    Statement* insertStatSmeStateStmt    = 0;
    Statement* insertStatRouteStateStmt  = 0;

    try
    {
        if (!(connection = ds.getConnection()))
            throw SQLException("Statistics: Failed to obtain DB connection!");

        insertStatSmsStmt   = connection->createStatement(insertStatSmsSql);
        insertStatSmeStmt   = connection->createStatement(insertStatSmeSql);
        insertStatRouteStmt = connection->createStatement(insertStatRouteSql);
        insertStatStateStmt      = connection->createStatement(insertStatStateSql);
        insertStatSmeStateStmt   = connection->createStatement(insertStatSmeStateSql);
        insertStatRouteStateStmt = connection->createStatement(insertStatRouteStateSql);

        if (!insertStatSmeStmt || !insertStatSmsStmt || !insertStatRouteStmt || 
            !insertStatStateStmt || !insertStatSmeStateStmt || !insertStatRouteStateStmt)
            throw SQLException("Statistics: Failed to create service statements!");

        insertStatSmsStmt->setUint32(1, period);
        insertStatSmsStmt->setInt32 (2, statGeneral[index].accepted);
        insertStatSmsStmt->setInt32 (3, statGeneral[index].rejected);
        insertStatSmsStmt->setInt32 (4, statGeneral[index].delivered);
        insertStatSmsStmt->setInt32 (5, statGeneral[index].failed);
        insertStatSmsStmt->setInt32 (6, statGeneral[index].rescheduled);
        insertStatSmsStmt->setInt32 (7, statGeneral[index].temporal);
        insertStatSmsStmt->setInt32 (8, statGeneral[index].peak_i);
        insertStatSmsStmt->setInt32 (9, statGeneral[index].peak_o);
        insertStatSmsStmt->executeUpdate();

        insertStatStateStmt->setUint32(1, period);
        IntHash<int>::Iterator it = statGeneral[index].errors.First();
        int ecError, eCounter;
        while (it.Next(ecError, eCounter))
        {
            insertStatStateStmt->setInt32(2, ecError);
            insertStatStateStmt->setInt32(3, eCounter);
            insertStatStateStmt->executeUpdate();
        }

        insertStatRouteStmt->setUint32(1, period);
        insertStatRouteStateStmt->setUint32(1, period);
        statByRoute[index].First();
        char* routeId = 0; SmsStat* routeStat = 0;
        while (statByRoute[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0') continue;
            insertStatRouteStmt->setString(2 , routeId);
            insertStatRouteStmt->setInt32 (3 , routeStat->accepted);
            insertStatRouteStmt->setInt32 (4 , routeStat->rejected);
            insertStatRouteStmt->setInt32 (5 , routeStat->delivered);
            insertStatRouteStmt->setInt32 (6 , routeStat->failed);
            insertStatRouteStmt->setInt32 (7 , routeStat->rescheduled);
            insertStatRouteStmt->setInt32 (8 , routeStat->temporal);
            insertStatRouteStmt->setInt32 (9 , routeStat->peak_i);
            insertStatRouteStmt->setInt32 (10, routeStat->peak_o);
            insertStatRouteStmt->executeUpdate();

            insertStatRouteStateStmt->setString(2, routeId);
            IntHash<int>::Iterator rit = routeStat->errors.First();
            int recError, reCounter;
            while (rit.Next(recError, reCounter))
            {
                insertStatRouteStateStmt->setInt32(3, recError);
                insertStatRouteStateStmt->setInt32(4, reCounter);
                insertStatRouteStateStmt->executeUpdate();
            }
            routeStat = 0;
        }

        insertStatSmeStmt->setUint32(1, period);
        insertStatSmeStateStmt->setUint32(1, period);
        statBySmeId[index].First();
        char* smeId = 0; SmsStat* smeStat = 0;
        while (statBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeStat || !smeId || smeId[0] == '\0') continue;
            insertStatSmeStmt->setString(2 , smeId);
            insertStatSmeStmt->setInt32 (3 , smeStat->accepted);
            insertStatSmeStmt->setInt32 (4 , smeStat->rejected);
            insertStatSmeStmt->setInt32 (5 , smeStat->delivered);
            insertStatSmeStmt->setInt32 (6 , smeStat->failed);
            insertStatSmeStmt->setInt32 (7 , smeStat->rescheduled);
            insertStatSmeStmt->setInt32 (8 , smeStat->temporal);
            insertStatSmeStmt->setInt32 (9 , smeStat->peak_i);
            insertStatSmeStmt->setInt32 (10, smeStat->peak_o);
            insertStatSmeStmt->executeUpdate();

            insertStatSmeStateStmt->setString(2, smeId);
            IntHash<int>::Iterator sit = smeStat->errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                insertStatSmeStateStmt->setInt32(3, secError);
                insertStatSmeStateStmt->setInt32(4, seCounter);
                insertStatSmeStateStmt->executeUpdate();
            }
            smeStat = 0;
        }

        connection->commit();
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, exc.what());
        try { if (connection) connection->rollback(); } catch (...) {}
    }

    if (insertStatSmeStmt)   delete insertStatSmeStmt;
    if (insertStatSmsStmt)   delete insertStatSmsStmt;
    if (insertStatRouteStmt) delete insertStatRouteStmt;
    if (insertStatStateStmt)      delete insertStatStateStmt;
    if (insertStatSmeStateStmt)   delete insertStatSmeStateStmt;
    if (insertStatRouteStateStmt) delete insertStatRouteStateStmt;
    if (connection) ds.freeConnection(connection);

    resetCounters(index);
}
void StatisticsManager::resetCounters(short index)
{
    statGeneral[index].Empty();
    statBySmeId[index].Empty();
    statByRoute[index].Empty();
}

}}
