
#include "StatisticsManager.h"

namespace smsc { namespace stat
{

StatisticsManager::StatisticsManager(DataSource& _ds)
    : Statistics(), ThreadedTask(),
        log(Logger::getCategory("smsc.stat.StatisticsManager")),
            ds(_ds), currentIndex(0), isStarted(false)
{
    resetCounters(0); resetCounters(1);
}
StatisticsManager::~StatisticsManager()
{
//    stop();
}

void StatisticsManager::updateAccepted(const char* srcSmeId)
{
    MutexGuard  incomingGuard(incomingLock);
    MutexGuard  switchGuard(switchLock);

    acceptedCount[currentIndex]++;
    if (srcSmeId && srcSmeId[0])
    {
        MutexGuard guard(smeStatLock);

        SmeStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        if (!stat) statBySmeId[currentIndex].Insert(srcSmeId, SmeStat(1, 0));
        else stat->sent++;
    }
}
void StatisticsManager::updateRejected(int errcode)
{
    MutexGuard  incomingGuard(outgoingLock);
    MutexGuard  switchGuard(switchLock);

    int* counter = finalizedByError[currentIndex].GetPtr(errcode);
    if (!counter) finalizedByError[currentIndex].Insert(errcode, 1);
    else (*counter)++;
}

void StatisticsManager::updateChanged(const char* dstSmeId,
    const char* routeId, int errcode = 0)
{
    MutexGuard  outgoingGuard(outgoingLock);
    MutexGuard  switchGuard(switchLock);

    finalizedCount[currentIndex]++;
    int* counter = finalizedByError[currentIndex].GetPtr(errcode);
    if (!counter) finalizedByError[currentIndex].Insert(errcode, 1);
    else (*counter)++;

    if (routeId)
    {
        counter = finalizedByRoute[currentIndex].GetPtr(routeId);
        if (!counter) finalizedByRoute[currentIndex].Insert(routeId, 1);
        else (*counter)++;
    }
    if (errcode == 0 && dstSmeId && dstSmeId[0])
    {
        MutexGuard guard(smeStatLock);

        SmeStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
        if (!stat) statBySmeId[currentIndex].Insert(dstSmeId, SmeStat(0, 1));
        else stat->received++;
    }
}
void StatisticsManager::updateScheduled()
{
    MutexGuard  scheduleGuard(scheduleLock);
    MutexGuard  switchGuard(switchLock);

    rescheduledCount[currentIndex]++;
}

int StatisticsManager::Execute()
{
    __trace2__("StatisticsManager::Execute() started (%d)", isStopping);
    isStarted = true;
    while (!isStopping)
    {
        int toSleep = calculateToSleep();
        __trace2__("StatisticsManager::Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        __trace__("StatisticsManager::Execute() >> End wait");

        flushCounters(switchCounters());
        doneEvent.Signal();
        __trace__("StatisticsManager::Execute() >> Flushed");
    }
    isStarted = false;
    exitEvent.Signal();
    __trace__("StatisticsManager::Execute() exited");
    return 0;
}

void StatisticsManager::stop()
{
    __trace2__("StatisticsManager::stop() called, started=%d", isStarted);
    ThreadedTask::stop();
    if (isStarted)
    {
        awakeEvent.Signal();
        __trace__("StatisticsManager::ctop() waiting finish ...");
        exitEvent.Wait();
    }
    __trace__("StatisticsManager::stop() exited");
}

void StatisticsManager::flushStatistics()
{
    MutexGuard flushGuard(flushLock);

    if (doneEvent.isSignaled()) doneEvent.Wait(0);
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
    time_t currTime = time(0) - 3600;
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
    return (nextTime-currTime)*1000;
}

const char* insertStatSmsSql = (const char*)
"INSERT INTO sms_stat_sms (period, received, finalized, rescheduled)\
 VALUES (:period, :received, :finalized, :rescheduled)";

const char* insertStatStateSql = (const char*)
"INSERT INTO sms_stat_state (period, errcode, counter)\
 VALUES (:period, :errcode, :counter)";

const char* insertStatSmeSql = (const char*)
"INSERT INTO sms_stat_sme (period, systemid, received, sent)\
 VALUES (:period, :systemid, :received, :sent)";

const char* insertStatRouteSql = (const char*)
"INSERT INTO sms_stat_route (period, routeid, processed)\
 VALUES (:period, :routeid, :processed)";

void StatisticsManager::flushCounters(short index)
{
    uint32_t period = calculatePeriod();

    Connection* connection = 0;

    Statement* insertStatSmeStmt    = 0;
    Statement* insertStatSmsStmt    = 0;
    Statement* insertStatRouteStmt  = 0;
    Statement* insertStatStateStmt  = 0;

    try
    {
        if (!(connection = ds.getConnection()))
            throw SQLException("Statistics: Failed to obtain DB connection!");

        insertStatSmeStmt   = connection->createStatement(insertStatSmeSql);
        insertStatSmsStmt   = connection->createStatement(insertStatSmsSql);
        insertStatRouteStmt = connection->createStatement(insertStatRouteSql);
        insertStatStateStmt = connection->createStatement(insertStatStateSql);

        if (!insertStatSmeStmt || !insertStatSmsStmt ||
            !insertStatRouteStmt || !insertStatStateStmt)
            throw SQLException("Statistics: Failed to create service statements!");

        insertStatSmsStmt->setUint32(1, period);
        insertStatSmsStmt->setInt32(2, acceptedCount[index]);
        insertStatSmsStmt->setInt32(3, finalizedCount[index]);
        insertStatSmsStmt->setInt32(4, rescheduledCount[index]);
        insertStatSmsStmt->executeUpdate();

        insertStatStateStmt->setUint32(1, period);
        IntHash<int>::Iterator it = finalizedByError[index].First();
        int fbeError, fbeCounter;
        while (it.Next(fbeError, fbeCounter))
        {
            insertStatStateStmt->setInt32(2, fbeError);
            insertStatStateStmt->setInt32(3, fbeCounter);
            insertStatStateStmt->executeUpdate();
        }

        insertStatRouteStmt->setUint32(1, period);
        finalizedByRoute[index].First();
        char* fbrId; int fbrCounter;
        while (finalizedByRoute[index].Next(fbrId, fbrCounter))
        {
            insertStatRouteStmt->setString(2, fbrId);
            insertStatRouteStmt->setInt32(3, fbrCounter);
            insertStatRouteStmt->executeUpdate();
        }

        insertStatSmeStmt->setUint32(1, period);
        statBySmeId[index].First();
        char* sbsId; SmeStat sbsStat;
        while (statBySmeId[index].Next(sbsId, sbsStat))
        {
            insertStatSmeStmt->setString(2, sbsId);
            insertStatSmeStmt->setInt32(3, sbsStat.received);
            insertStatSmeStmt->setInt32(4, sbsStat.sent);
            insertStatSmeStmt->executeUpdate();
        }

        connection->commit();
    }
    catch (Exception& exc)
    {
        if (connection) connection->rollback();
        log.error(exc.what());
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
    acceptedCount[index] = 0;
    finalizedCount[index] = 0;
    rescheduledCount[index] = 0;

    statBySmeId[index].Empty();
    finalizedByError[index].Empty();
    finalizedByRoute[index].Empty();
}

}}
