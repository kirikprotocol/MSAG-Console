
#include "StatisticsManager.h"

namespace smsc { namespace infosme
{

StatisticsManager::StatisticsManager(Connection* connection)
    : Statistics(), Thread(), logger(Logger::getInstance("smsc.infosme.StatisticsManager")),
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
        stat = statistics[currentIndex].GetPtr(task_id);
        smsc_log_debug(logger, "StatisticsManager::incGenerated::: modify statistic in hash: task_id=%s,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", task_id,stat->generated,stat->delivered,stat->retried,stat->failed);
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
        stat = statistics[currentIndex].GetPtr(task_id);
        smsc_log_debug(logger, "StatisticsManager::incDelivered::: modify statistic in hash: task_id=%s,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", task_id,stat->generated,stat->delivered,stat->retried,stat->failed);
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
        stat = statistics[currentIndex].GetPtr(task_id);
        smsc_log_debug(logger, "StatisticsManager::incRetried::: modify statistic in hash: task_id=%s,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", task_id,stat->generated,stat->delivered,stat->retried,stat->failed);
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
        stat = statistics[currentIndex].GetPtr(task_id);
        smsc_log_debug(logger, "StatisticsManager::incFailed::: modify statistic in hash: task_id=%s,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", task_id,stat->generated,stat->delivered,stat->retried,stat->failed);
    }
}

int StatisticsManager::Execute()
{
    while (!bNeedExit)
    {
        int toSleep = 60*1000; // = calculateToSleep();
        smsc_log_debug(logger, "StatisticsManager:: Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "StatisticsManager:: End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "StatisticsManager:: Statistics flushed");
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

const char* DELETE_TASK_STAT_STATE_SQL = (const char*)
"DELETE FROM INFOSME_TASKS_STAT WHERE task_id=:task_id";

void StatisticsManager::delStatistics(std::string taskId)
{
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0') return;
    
    flushStatistics();
    
    smsc_log_debug(logger, "StatisticsManager:: Deleting statistics for task '%s'", task_id);

    try
    {
        std::auto_ptr<Statement> statementGuard(connection->createStatement(DELETE_TASK_STAT_STATE_SQL));
        Statement* statement = statementGuard.get();
        if (!statement)
            throw Exception("Failed to obtain statement for statistics delete");
        
        statement->setString(1, task_id);
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
        smsc_log_error(logger, "Error occurred during statistics deleting for task '%s'. "
                     "Details: %s", task_id, exc.what());
    }
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
    //    if (!bExternalFlush) currTime -= 600 /*3600*/;
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
    smsc_log_debug(logger, "Flushing statistics for period: period=%lu / time=%lu, index=%d", period, time(NULL),index);

    try
    {
        std::auto_ptr<Statement> statement(connection->createStatement(INSERT_TASK_STAT_STATE_SQL));
        if (!statement.get())
            throw Exception("Failed to obtain statement for statistics update");
        
        statement->setUint32(2, period);
        statistics[index].First();
        char* task_id = 0; TaskStat stat;
        smsc_log_debug(logger, "StatisticsManager::flushCounters::: try bypass statistics hash");
        while (statistics[index].Next(task_id, stat))
        {
          if (!task_id || task_id[0] == '\0') continue;
          smsc_log_debug(logger, "StatisticsManager::flushCounters::: insert statistic: task_id=%s,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", task_id,stat.generated,stat.delivered,stat.retried,stat.failed);
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
