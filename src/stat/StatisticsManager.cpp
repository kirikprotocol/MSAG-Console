
#include "StatisticsManager.h"

namespace smsc { namespace stat
{

StatisticsManager::StatisticsManager(DataSource& _ds)
    : Statistics(), ThreadedTask(),
        logger(Logger::getCategory("smsc.stat.StatisticsManager")),
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
    if (srcSmeId && srcSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        if (stat) stat->accepted++;
        else statBySmeId[currentIndex].Insert(srcSmeId, SmsStat(1));
    }
    if (routeId && routeId[0])
    {
        SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) stat->accepted++;
        else statByRoute[currentIndex].Insert(routeId, SmsStat(1));
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
            else {
                stat->failed++;
                addError(stat->errors, errcode);
            }
        }
        else {
            SmsStat newStat(0, 0, (errcode) ? 0:1, (errcode) ? 1:0, 0, 0); // delivered or failed
            if (errcode != 0) addError(newStat.errors, errcode);
            statBySmeId[currentIndex].Insert(dstSmeId, newStat);
        }
    }
    
    if (routeId && routeId[0])
    {
        SmsStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) {
            if (errcode == 0) stat->delivered++;
            else {
                stat->failed++;
                addError(stat->errors, errcode);
            }
        }
        else {
            SmsStat newStat(0, 0, (errcode) ? 0:1, (errcode) ? 1:0, 0, 0); // delivered or failed
            if (errcode != 0) addError(newStat.errors, errcode);
            statByRoute[currentIndex].Insert(routeId, newStat);
        }
    }
    
    if (errcode == 0) statGeneral[currentIndex].delivered++;
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
    logger.debug("Execute() started (%d)", isStopping);
    isStarted = true; bExternalFlush = false;
    while (!isStopping)
    {
        int toSleep = calculateToSleep();
        logger.debug("Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        logger.debug("Execute() >> End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        logger.debug("Execute() >> Flushed");
    }
    isStarted = false;
    exitEvent.Signal();
    logger.debug("Execute() exited");
    return 0;
}

void StatisticsManager::stop()
{
    MutexGuard guard(stopLock);

    logger.debug("stop() called, started=%d", isStarted);
    ThreadedTask::stop();
    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
        logger.debug("stop() waiting finish ...");
        exitEvent.Wait();
    }
    logger.debug("stop() exited");
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
"INSERT INTO sms_stat_sms (period, accepted, rejected, delivered, rescheduled, temporal) "
"VALUES (:period, :accepted, :rejected, :delivered, :failed, :rescheduled, :temporal)";

const char* insertStatSmeSql = (const char*)
"INSERT INTO sms_stat_sme (period, systemid, accepted, rejected, delivered, rescheduled, temporal)\
 VALUES (:period, :systemid, :accepted, :rejected, :delivered, :rescheduled, :temporal)";

const char* insertStatRouteSql = (const char*)
"INSERT INTO sms_stat_route (period, routeid, accepted, rejected, delivered, rescheduled, temporal)\
 VALUES (:period, :routeid, :accepted, :rejected, :delivered, :rescheduled, :temporal)";

const char* insertStatStateSql = (const char*)
"INSERT INTO sms_stat_state (period, errcode, counter) "
"VALUES (:period, :errcode, :counter)";

void StatisticsManager::flushCounters(short index)
{
    uint32_t period = calculatePeriod();

    logger.debug("Flushing statistics for period: %d / %d", period, time(0));

    Connection* connection = 0;

    Statement* insertStatSmeStmt    = 0;
    Statement* insertStatSmsStmt    = 0;
    Statement* insertStatRouteStmt  = 0;
    Statement* insertStatStateStmt  = 0;

    try
    {
        if (!(connection = ds.getConnection()))
            throw SQLException("Statistics: Failed to obtain DB connection!");

        insertStatSmsStmt   = connection->createStatement(insertStatSmsSql);
        insertStatSmeStmt   = connection->createStatement(insertStatSmeSql);
        insertStatRouteStmt = connection->createStatement(insertStatRouteSql);
        insertStatStateStmt = connection->createStatement(insertStatStateSql);

        if (!insertStatSmeStmt || !insertStatSmsStmt ||
            !insertStatRouteStmt || !insertStatStateStmt)
            throw SQLException("Statistics: Failed to create service statements!");

        insertStatSmsStmt->setUint32(1, period);
        insertStatSmsStmt->setInt32 (2, statGeneral[index].accepted);
        insertStatSmsStmt->setInt32 (3, statGeneral[index].rejected);
        insertStatSmsStmt->setInt32 (4, statGeneral[index].delivered);
        insertStatSmsStmt->setInt32 (5, statGeneral[index].failed);
        insertStatSmsStmt->setInt32 (6, statGeneral[index].rescheduled);
        insertStatSmsStmt->setInt32 (7, statGeneral[index].temporal);
        insertStatSmsStmt->executeUpdate();

        insertStatStateStmt->setUint32(1, period);
        IntHash<int>::Iterator it = statGeneral[index].errors.First();
        int fbeError, fbeCounter;
        while (it.Next(fbeError, fbeCounter))
        {
            insertStatStateStmt->setInt32(2, fbeError);
            insertStatStateStmt->setInt32(3, fbeCounter);
            insertStatStateStmt->executeUpdate();
        }

        insertStatRouteStmt->setUint32(1, period);
        statByRoute[index].First();
        char* routeId = 0; SmsStat routeStat;
        while (statByRoute[index].Next(routeId, routeStat))
        {
            if (!routeId || routeId[0] == '\0') continue;
            insertStatRouteStmt->setString(2, routeId);
            insertStatRouteStmt->setInt32 (3, routeStat.accepted);
            insertStatRouteStmt->setInt32 (4, routeStat.rejected);
            insertStatRouteStmt->setInt32 (5, routeStat.delivered);
            insertStatRouteStmt->setInt32 (6, routeStat.failed);
            insertStatRouteStmt->setInt32 (7, routeStat.rescheduled);
            insertStatRouteStmt->setInt32 (8, routeStat.temporal);
            insertStatRouteStmt->executeUpdate();

            // TODO: Add errors set flush here
        }

        insertStatSmeStmt->setUint32(1, period);
        statBySmeId[index].First();
        char* smeId = 0; SmsStat smeStat;
        while (statBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeId || smeId[0] == '\0') continue;
            insertStatSmeStmt->setString(2, smeId);
            insertStatSmeStmt->setInt32 (3, smeStat.accepted);
            insertStatSmeStmt->setInt32 (4, smeStat.rejected);
            insertStatSmeStmt->setInt32 (5, smeStat.delivered);
            insertStatSmeStmt->setInt32 (6, smeStat.failed);
            insertStatSmeStmt->setInt32 (7, smeStat.rescheduled);
            insertStatSmeStmt->setInt32 (8, smeStat.temporal);
            insertStatSmeStmt->executeUpdate();

            // TODO: Add errors set flush here
        }

        connection->commit();
    }
    catch (Exception& exc)
    {
        if (connection) connection->rollback();
        logger.error(exc.what());
    }

    if (insertStatSmeStmt) delete insertStatSmeStmt;
    if (insertStatSmsStmt) delete insertStatSmsStmt;
    if (insertStatRouteStmt) delete insertStatRouteStmt;
    if (insertStatStateStmt) delete insertStatStateStmt;
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
