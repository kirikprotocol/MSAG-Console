
#include "StatisticsManager.h"

namespace smsc { namespace infosme
{

StatisticsManager::StatisticsManager(Connection* connection)
    : Statistics(), Thread(), logger(Logger::getCategory("smsc.infosme.StatisticsManager")),
            connection(connection), currentIndex(0), 
                bExternalFlush(false), bStarted(false), bNeedExit(false)
{
}
StatisticsManager::~StatisticsManager()
{
    Stop();
}

bool StatisticsManager::getStatistics(std::string taskId, TaskStat& stat)
{
    const char* task_id = taskId.c_str();
    if (task_id && task_id[0])
    {
        MutexGuard guard(switchLock);
        
        TaskStat* st = statistics[currentIndex].GetPtr(task_id);
        if (st) {
            stat = *st;
            return true;
        }
    }
    return false;
}
void StatisticsManager::incGenerated(std::string taskId, unsigned inc)
{
    const char* task_id = taskId.c_str();
    if (task_id && task_id[0])
    {
        MutexGuard guard(switchLock);
        
        TaskStat* stat = statistics[currentIndex].GetPtr(task_id);
        if (!stat) statistics[currentIndex].Insert(task_id, TaskStat(inc, 0, 0, 0));
        else stat->generated += inc;
    }
}
void StatisticsManager::incDelivered(std::string taskId, unsigned inc)
{
    const char* task_id = taskId.c_str();
    if (task_id && task_id[0])
    {
        MutexGuard  guard(switchLock);
        
        TaskStat* stat = statistics[currentIndex].GetPtr(task_id);
        if (!stat) statistics[currentIndex].Insert(task_id, TaskStat(0, inc, 0, 0));
        else stat->delivered += inc;
    }
}
void StatisticsManager::incRetried(std::string taskId, unsigned inc)
{
    const char* task_id = taskId.c_str();
    if (task_id && task_id[0])
    {
        MutexGuard guard(switchLock);
        
        TaskStat* stat = statistics[currentIndex].GetPtr(task_id);
        if (!stat) statistics[currentIndex].Insert(task_id, TaskStat(0, 0, inc, 0));
        else stat->retried += inc;
    }
}
void StatisticsManager::incFailed(std::string taskId, unsigned inc)
{
    const char* task_id = taskId.c_str();
    if (task_id && task_id[0])
    {
        MutexGuard guard(switchLock);
        
        TaskStat* stat = statistics[currentIndex].GetPtr(task_id);
        if (!stat) statistics[currentIndex].Insert(task_id, TaskStat(0, 0, 0, inc));
        else stat->failed += inc;
    }
}

int StatisticsManager::Execute()
{
    while (!bNeedExit)
    {
        int toSleep = calculateToSleep();
        logger.debug("Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        logger.debug("End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        logger.debug("Statistics flushed");
    }
    exitEvent.Signal();
    return 0;
}

void StatisticsManager::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        bExternalFlush = false;
        bNeedExit = false;
        awakeEvent.Wait(0);
        Thread::Start();
        bStarted = true;
        logger.info("Started.");
    }
}
void StatisticsManager::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        logger.info("Stopping ...");
        bExternalFlush = true;
        bNeedExit = true;
        awakeEvent.Signal();
        exitEvent.Wait();
        bStarted = false;
        logger.info("Stoped.");
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

const char* INSERT_TASK_STAT_STATE_ID = "INSERT_TASK_STAT_STATE_ID";
const char* INSERT_TASK_STAT_STATE_SQL = (const char*)
"INSERT INTO INFOSME_TASKS_STAT (task_id, period, generated, delivered, retried, failed) "
"VALUES (:task_id, :period, :generated, :delivered, :retried, :failed)";

void StatisticsManager::flushCounters(short index)
{
    uint32_t period = calculatePeriod();

    try
    {
        Statement* statement = connection->getStatement(INSERT_TASK_STAT_STATE_ID, 
                                                        INSERT_TASK_STAT_STATE_SQL);
        if (!statement)
            throw Exception("Failed to obtain statement for statistic update");

        logger.debug("Flushing statistics for period: %lld / %lld", period, time(NULL));
        statement->setUint32(2, period);
        statistics[index].First();
        char* task_id = 0; TaskStat stat;
        while (statistics[index].Next(task_id, stat))
        {
            statement->setString(1, task_id);
            statement->setUint32(3, stat.generated);
            statement->setUint32(4, stat.delivered);
            statement->setUint32(5, stat.retried);
            statement->setUint32(6, stat.failed);
            
            statement->executeUpdate();
        }
        
        connection->commit();
    }
    catch (Exception& exc)
    {
        if (connection) connection->rollback();
        logger.error("Error occurred during statistics flushing. Details: %s", exc.what());
    }

    statistics[index].Empty();
}

}}
