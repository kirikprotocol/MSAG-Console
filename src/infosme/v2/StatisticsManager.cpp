#include "StatisticsManager.h"
#include "Task.h"

namespace smsc { namespace infosme
{

StatisticsManager::StatisticsManager(const std::string& argLocation,InfoSmeAdmin* argAdmin)
    : Statistics(), Thread(), logger(Logger::getInstance("smsc.infosme.StatisticsManager")),
    currentHour(-100),
            currentIndex(0), bExternalFlush(false), bStarted(false), bNeedExit(false)
{
  admin=argAdmin;
  storeLocation=argLocation;
  if(storeLocation.length())
  {
    if(storeLocation[storeLocation.length()-1]!='/')storeLocation+='/';
  }else
  {
    storeLocation="./";
  }
#ifdef INTHASH_USAGE_CHECKING
  statistics[0].reset( new IntHash< TaskStat >(SMSCFILELINE) );
  statistics[1].reset( new IntHash< TaskStat >(SMSCFILELINE) );
#else
  statistics[0].reset( new IntHash< TaskStat >() );
  statistics[1].reset( new IntHash< TaskStat >() );
#endif
}
StatisticsManager::~StatisticsManager()
{
    Stop();
}

bool StatisticsManager::getStatistics(uint32_t taskId, TaskStat& stat)
{
  MutexGuard guard(switchLock);
  
  TaskStat* st = statistics[currentIndex]->GetPtr(taskId);
  if (st)
  {
    stat = *st;
    return true;
  }
  return false;
}
void StatisticsManager::incGenerated(uint32_t taskId, unsigned inc)
{
  MutexGuard guard(switchLock);
  
  TaskStat* stat = statistics[currentIndex]->GetPtr(taskId);
  if (!stat) statistics[currentIndex]->Insert(taskId, TaskStat(inc, 0, 0, 0));
  else stat->generated += inc;
  stat = statistics[currentIndex]->GetPtr(taskId);
  smsc_log_debug(logger, "StatisticsManager::incGenerated::: modify statistic in hash: task_id=%d,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", taskId,stat->generated,stat->delivered,stat->retried,stat->failed);
}
void StatisticsManager::incDelivered(uint32_t taskId, unsigned inc)
{
  MutexGuard  guard(switchLock);
  
  TaskStat* stat = statistics[currentIndex]->GetPtr(taskId);
  if (!stat) statistics[currentIndex]->Insert(taskId, TaskStat(0, inc, 0, 0));
  else stat->delivered += inc;
  stat = statistics[currentIndex]->GetPtr(taskId);
  smsc_log_debug(logger, "StatisticsManager::incDelivered::: modify statistic in hash: task_id=%d,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", taskId,stat->generated,stat->delivered,stat->retried,stat->failed);
}
void StatisticsManager::incRetried(uint32_t taskId, unsigned inc)
{
  MutexGuard guard(switchLock);
  
  TaskStat* stat = statistics[currentIndex]->GetPtr(taskId);
  if (!stat) statistics[currentIndex]->Insert(taskId, TaskStat(0, 0, inc, 0));
  else stat->retried += inc;
  stat = statistics[currentIndex]->GetPtr(taskId);
  smsc_log_debug(logger, "StatisticsManager::incRetried::: modify statistic in hash: task_id=%d,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", taskId,stat->generated,stat->delivered,stat->retried,stat->failed);
}
void StatisticsManager::incFailed(uint32_t taskId, unsigned inc)
{
  MutexGuard guard(switchLock);
  
  TaskStat* stat = statistics[currentIndex]->GetPtr(taskId);
  if (!stat) statistics[currentIndex]->Insert(taskId, TaskStat(0, 0, 0, inc));
  else stat->failed += inc;
  stat = statistics[currentIndex]->GetPtr(taskId);
  smsc_log_debug(logger, "StatisticsManager::incFailed::: modify statistic in hash: task_id=%d,stat.generated=%d,stat.delivered=%d,stat.retried=%d,stat.failed=%d", taskId,stat->generated,stat->delivered,stat->retried,stat->failed);
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

void StatisticsManager::delStatistics(uint32_t taskId)
{
/*
  char task_id[32];
  sprintf(task_id,"%u",taskId);
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
    */
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
    return (int)(((nextTime-currTime)*1000)+1);
}

const char* INSERT_TASK_STAT_STATE_ID = "INSERT_TASK_STAT_STATE_ID";
const char* INSERT_TASK_STAT_STATE_SQL = (const char*)
"INSERT INTO INFOSME_TASKS_STAT (task_id, period, generated, delivered, retried, failed) "
"VALUES (:task_id, :period, :generated, :delivered, :retried, :failed)";

void StatisticsManager::flushCounters(short index)
{
    //uint32_t period = calculatePeriod();
    smsc_log_debug(logger, "Flushing statistics for period: time=%lu, index=%d", time(NULL),index);

    try
    {
      time_t now=time(NULL);
      struct tm t;
      localtime_r(&now,&t);
      char dirName[64];
      sprintf(dirName,"%04d%02d%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday);
      std::string fullPath=storeLocation;
      fullPath+=dirName;
      if(!File::Exists(fullPath.c_str()))
      {
        File::MkDir(fullPath.c_str());
      }
      char fileName[32];
      sprintf(fileName,"%02d.csv",t.tm_hour);
      fullPath+='/';
      fullPath+=fileName;
      int taskId;
      TaskStat* st;
      IntHash<TaskStat>::Iterator it=statistics[index]->First();
      char buf[128];
      std::string line;
      while(it.Next(taskId,st))
      {
        if(!currentFile.isOpened())
        {
          if(File::Exists(fullPath.c_str()))
          {
            currentFile.Append(fullPath.c_str());
          }else
          {
            currentFile.WOpen(fullPath.c_str());
            const char* header="TASK_ID,TASK_NAME,MINUTE,GENERATED,DELIVERED,RETRIED,FAILED\n";
            currentFile.Write(header,strlen(header));
          }
          currentHour=t.tm_hour;
        }
        sprintf(buf,"%u,",taskId);
        line=buf;
        line+='"';
          {
              TaskGuard tg(admin->getTask(taskId));
              if (tg.get()) line += tg->getName();
          }
        line+="\",";
        sprintf(buf,"%d,%d,%d,%d,%d\n",t.tm_min,st->generated,st->delivered,st->retried,st->failed);
        line+=buf;
        currentFile.Write(line.c_str(),line.length());
      }
      if(currentFile.isOpened())
      {
        currentFile.Flush();
      }
      if(currentHour!=t.tm_hour && currentFile.isOpened())
      {
        currentFile.Close();
      }

/*
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
        */
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Error occurred during statistics flushing. Details: %s", exc.what());
    }

    statistics[index]->Empty();
}

}}
