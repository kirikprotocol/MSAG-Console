
#include "StatisticsManager.h"

namespace smsc { namespace mcisme
{

StatisticsManager::StatisticsManager(Connection* connection)
    : Statistics(), Thread(), logger(Logger::getInstance("smsc.mcisme.StatisticsManager")),
            connection(connection), currentIndex(0), 
                bExternalFlush(false), bStarted(false), bNeedExit(false)
{
}
StatisticsManager::~StatisticsManager()
{
    Stop();
}

EventsStat StatisticsManager::getStatistics()
{
    MutexGuard guard(switchLock);
    return statistics[currentIndex];
}
void StatisticsManager::incMissed(unsigned inc)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].missed += inc;
}
void StatisticsManager::incDelivered(unsigned inc)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].delivered += inc;
}
void StatisticsManager::incFailed(unsigned inc)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].failed += inc;
}
void StatisticsManager::incNotified(unsigned inc)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].notified += inc;
}

int StatisticsManager::Execute()
{
    while (!bNeedExit)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "Statistics flushed");
    }
    exitEvent.Signal();
    return 0;
}

void StatisticsManager::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        smsc_log_info(logger, "Starting ...");
        bExternalFlush = false;
        bNeedExit = false;
        awakeEvent.Wait(0);
        Thread::Start();
        bStarted = true;
        smsc_log_info(logger, "Started.");
    }
}
void StatisticsManager::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        smsc_log_info(logger, "Stopping ...");
        bExternalFlush = true;
        bNeedExit = true;
        awakeEvent.Signal();
        exitEvent.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
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
    return  (tmCT.tm_year+1900)*1000000+(tmCT.tm_mon+1)*10000+(tmCT.tm_mday)*100+tmCT.tm_hour;
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

const char* INSERT_EVENTS_STAT_STATE_ID = "INSERT_EVENTS_STAT_STATE_ID";
const char* INSERT_EVENTS_STAT_STATE_SQL = (const char*)
"INSERT INTO MCISME_STAT (period, missed, delivered, failed, notified) "
"VALUES (:period, :missed, :delivered, :failed, :notified)";

void StatisticsManager::flushCounters(short index)
{
    if (statistics[index].isEmpty()) return;

    uint32_t period = calculatePeriod();
    smsc_log_debug(logger, "Flushing statistics for period: %lu / %lu", period, time(NULL));

    try
    {
        Statement* statement = connection->getStatement(INSERT_EVENTS_STAT_STATE_ID, 
                                                        INSERT_EVENTS_STAT_STATE_SQL);
        if (!statement)
            throw Exception("Failed to obtain statement for statistics update");
        
        statement->setUint32(1, period);
        statement->setUint32(2, statistics[index].missed);
        statement->setUint32(3, statistics[index].delivered);
        statement->setUint32(4, statistics[index].failed);
        statement->setUint32(5, statistics[index].notified);

        statement->executeUpdate();
        connection->commit();
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction (statistics). "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction (statistics).");
        }
        smsc_log_error(logger, "Error occurred during statistics flushing. Details: %s", exc.what());
    }

    statistics[index].Empty();
}

}}
