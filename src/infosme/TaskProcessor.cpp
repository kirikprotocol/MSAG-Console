#include "TaskProcessor.h"
#include "MessageSender.h"
#include "FinalStateSaver.h"
#include "SmscConnector.h"
#include <exception>
#include <list>
#include <sstream>
#include <util/timeslotcounter.hpp>
#include <util/config/region/RegionFinder.hpp>

#ident "@(#)$Id$"

extern bool isMSISDNAddress(const char* string);

namespace smsc {
namespace infosme {

RetryPolicies TaskProcessor::retryPlcs;

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config) :
TaskProcessorAdapter(), InfoSmeAdmin(), Thread(),
log_(Logger::getInstance("smsc.infosme.TaskProcessor")), 
bStarted(false),
bNeedExit(false),
messageSender(0), 
responseWaitTime(0),
receiptWaitTime(0),
mappingRollTime(0),
mappingMaxChanges(0),
dsStatConnection(0),
statistics(0), protocolId(0), svcType(0), address(0),
unrespondedMessagesMax(1),
unrespondedMessagesSleep(10)
{
    smsc_log_info(log_, "Loading ...");

    address = config->getString("Address");
    if (!address || !isMSISDNAddress(address))
        throw ConfigException("Address string '%s' is invalid", address ? address:"-");
    
    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
    responseWaitTime = parseTime(config->getString("responceWaitTime"));
    if (responseWaitTime <= 0) 
        throw ConfigException("Invalid value for 'responceWaitTime' parameter.");
    receiptWaitTime = parseTime(config->getString("receiptWaitTime"));
    if (receiptWaitTime <= 0) 
        throw ConfigException("Invalid value for 'receiptWaitTime' parameter.");

    switchTimeout = config->getInt("tasksSwitchTimeout");
    if (switchTimeout <= 0) 
        throw ConfigException("Task switch timeout should be positive");

    try { unrespondedMessagesMax = config->getInt("unrespondedMessagesMax"); } catch (...) {};
    if (unrespondedMessagesMax <= 0) {
      unrespondedMessagesMax = 1;
      smsc_log_warn(log_, "Parameter 'unrespondedMessagesMax' value is invalid. Using default %d",
                    unrespondedMessagesMax);
    }
    if (unrespondedMessagesMax > 500) {
      smsc_log_warn(log_, "Parameter 'unrespondedMessagesMax' value '%d' is too big. "
                    "The preffered max value is 500", unrespondedMessagesMax);
    }
    
    try { unrespondedMessagesSleep = config->getInt("unrespondedMessagesSleep"); } catch (...) {};
    if (unrespondedMessagesSleep <= 0) {
      unrespondedMessagesSleep = 10;
      smsc_log_warn(log_, "'unrespondedMessagesSleep' value is invalid. Using default %dms",
                    unrespondedMessagesSleep);
    }
    if (unrespondedMessagesSleep > 500) {
      smsc_log_warn(log_, "Parameter 'unrespondedMessagesSleep' value '%d' is too big. "
                    "The preffered max value is 500ms", unrespondedMessagesSleep);
    }

    std::auto_ptr<ConfigView> retryPlcCfg(config->getSubConfig("RetryPolicies"));
    retryPlcs.Load(retryPlcCfg.get());

    std::auto_ptr<ConfigView> tasksThreadPoolCfgGuard(config->getSubConfig("TasksThreadPool"));
    taskManager.init(tasksThreadPoolCfgGuard.get());   // loads up thread pool for tasks
    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("EventsThreadPool"));
    eventManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
    provider.init(providerCfgGuard.get());
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("systemDataSource"));

    smsc_log_info(log_, "Loading tasks ...");
    std::auto_ptr<ConfigView> tasksCfgGuard(config->getSubConfig("Tasks"));
    ConfigView* tasksCfg = tasksCfgGuard.get();
    std::auto_ptr< std::set<std::string> > setGuard(tasksCfg->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();

    storeLocation=config->getString("storeLocation");
    if(storeLocation.length())
    {
      if(*storeLocation.rbegin()!='/')
      {
        storeLocation+='/';
      }
    }

    finalStateSaver_.reset( new FinalStateSaver(storeLocation) );

    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* taskId = (const char *)i->c_str();
            if (!taskId || taskId[0] == '\0')
                throw ConfigException("Task id empty or wasn't specified");
            smsc_log_info(log_, "Loading task '%s' ...", taskId);
            
            std::auto_ptr<ConfigView> taskConfigGuard(tasksCfg->getSubConfig(taskId));
            ConfigView* taskConfig = taskConfigGuard.get();

            bool delivery = false;
            try { delivery = taskConfig->getBool("delivery"); }
            catch (ConfigException& ce) { delivery = false; }

            std::string location=storeLocation+taskId;
            if(!buf::File::Exists(location.c_str()))
            {
              buf::File::MkDir(location.c_str());
            }
            
            DataSource* taskDs = 0;
            if (!delivery)
            {
                const char* dsId = taskConfig->getString("dsId");
                if (!dsId || dsId[0] == '\0')
                    throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                          taskId);
                taskDs = provider.getDataSource(dsId);
                if (!taskDs)
                    throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                          dsId, taskId);
            }
            uint32_t taskIdVal=atoi(taskId);
            Task* task = new Task(taskConfig, taskIdVal, location, taskDs, finalStateSaver_.get() );
            if (task && !putTask(task)) {
                task->finalize();
                throw ConfigException("Failed to add task. Task with id '%s' already registered.",
                                      taskId);
            }
        }
        catch (ConfigException& exc)
        {
            smsc_log_error(log_, "Load of tasks failed ! Config exception: %s", exc.what());
            throw;
        }
    }
    smsc_log_info(log_, "Tasks loaded.");

    smsc_log_info(log_, "Loading task schedules ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Schedules"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(this, schedulerCfg);
    smsc_log_info(log_, "Task schedules loaded.");
    
    smsc_log_info(log_, "Load success.");

    // jstore.Init((storeLocation+"mapping.bin").c_str(),config->getInt("mappingRollTime"),config->getInt("mappingMaxChanges"));
    mappingRollTime = config->getInt("mappingRollTime");
    mappingMaxChanges = config->getInt("mappingMaxChanges");
    
    statistics = new StatisticsManager(config->getString("statStoreLocation"),this);
    if (statistics) statistics->Start();
    scheduler.Start();
}
TaskProcessor::~TaskProcessor()
{
    // jstore.Stop();
  scheduler.Stop();
  this->Stop();
  taskManager.Stop();
  eventManager.Stop();
  
  {
      MutexGuard guard(tasksLock);
      int key = 0; Task* task = 0; 
      IntHash<Task*>::Iterator it=tasks.First();
      while (it.Next(key, task))
          if (task) task->shutdown();
      tasks.Empty();
  }

  taskManager.shutdown();
  eventManager.shutdown();

  if (statistics) delete statistics;

}

bool TaskProcessor::putTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    if (tasks.Exist(task->getId())) return false;
    tasks.Insert(task->getId(), task);
    awake.Signal();
    return true;
}
bool TaskProcessor::addTask(Task* task)
{
    __require__(task);
    
    if (hasTask(task->getId())) return false;
    /*if (task->canGenerateMessages())*/
    return putTask(task);
}
bool TaskProcessor::remTask(uint32_t taskId)
{
  Task* task = 0;
  {
    MutexGuard guard(tasksLock);
    if (!tasks.Exist(taskId)) return false;
    task = tasks.Get(taskId);
    tasks.Delete(taskId);
    if (!task) return false;
    awake.Signal();
  }
  if (task) task->shutdown();
  return true;
}
bool TaskProcessor::delTask(uint32_t taskId)
{
    Task* task = 0;
    {
        MutexGuard guard(tasksLock);
        if (!tasks.Exist(taskId)) return false;
        task = tasks.Get(taskId);
        tasks.Delete(taskId);
        if (!task) return false;
        awake.Signal();
    }
    if (task) task->destroy();
    return (task) ? true:false;
}
bool TaskProcessor::hasTask(uint32_t taskId)
{
    MutexGuard guard(tasksLock);

    return tasks.Exist(taskId);
}
TaskGuard TaskProcessor::getTask(uint32_t taskId)
{
    MutexGuard guard(tasksLock);

    if (!tasks.Exist(taskId)) return TaskGuard(0);
    Task* task = tasks.Get(taskId);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}

void TaskProcessor::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    
    int key = 0; 
    Task* task = 0;
    IntHash<Task*>::Iterator it=tasks.First();
    while (it.Next(key, task))
        if (task) task->resetWaiting();
}
void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        smsc_log_info(log_, "Starting ...");

        if (!isMessageSenderAssigned()) {
            smsc_log_error(log_, "Failed to start processing. Message sender is undefined.");
            return;
        }
        resetWaitingTasks();
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
        smsc_log_info(log_, "Started.");
    }
}
void TaskProcessor::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        smsc_log_info(log_, "Stopping ...");
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        smsc_log_info(log_, "Stoped.");
    }
}
int TaskProcessor::Execute()
{
    Array<TaskGuard *> taskGuards;

    while (!bNeedExit)
    {
        time_t currentTime = time(NULL);
        finalStateSaver_->checkRoll(currentTime);
        
        {
            MutexGuard guard(tasksLock);
            int key = 0;
            Task* task = 0;
            IntHash<Task*>::Iterator it=tasks.First();
            while (!bNeedExit && it.Next(key, task))
                if (task && task->isReady(currentTime, true)) {
                    taskGuards.Push(new TaskGuard(task));
                    task->currentPriorityFrameCounter = 0;
                    task->resetSuspendedRegions();
                }
        }

        int processed = 0;
        while (taskGuards.Count()>0)
        {
            TaskGuard* taskGuard = 0;
            taskGuards.Shift(taskGuard);
            if (!taskGuard) continue;

            do {
                if (bNeedExit) break;
                Task* task = taskGuard->get();
                if (!task) break;
                if (task->isFinalizing() || !task->isEnabled()) break;
                const unsigned taskProcessed = processTask(task);
                if ( ! taskProcessed ) {
                    // no one message in task is processed
                    task->currentPriorityFrameCounter = task->getPriority();
                    if (!task->isEnabled()) task->setEnabled(false);
                }
                processed += taskProcessed;
            } while ( false );
            delete taskGuard;
        }

        if (bNeedExit) break;

        processWaitingEvents(time(NULL)); // ?? or time(NULL)
        if (!bNeedExit && processed <= 0) {
            smsc_log_info(log_,"TaskProc: processed=%d waiting %d",processed,switchTimeout);
            awake.Wait(switchTimeout);
        }
    }
    exited.Signal();
    return 0;
}


unsigned TaskProcessor::processTask(Task* task)
{
    __require__(task);
    unsigned res = 0;
    smsc_log_debug(log_, "TaskProc::processTask(%d) taskPrio(cur/tot)=%d/%d",
                   task->getId(), task->currentPriorityFrameCounter, task->getPriority() );
    while ( task->currentPriorityFrameCounter < task->getPriority() ) {

        ++task->currentPriorityFrameCounter;

        Message message;
        if (!task->getNextMessage(message)) continue;

        MutexGuard msGuard(messageSenderLock);
        if (! messageSender) {
            smsc_log_error(log_, "No messageSender defined !!!");
            break;
        }
        if ( messageSender->send(task,message) ) {
            ++res;
        }
    }
    return res;
}


void TaskProcessor::processWaitingEvents(time_t tm)
{
    MutexGuard msGuard(messageSenderLock);
    messageSender->processWaitingEvents(tm);
}


bool TaskProcessor::processResponse( const TaskMsgId& tmIds,
                                     const ResponseData& rd,
                                     bool  internal,
                                     bool  receipted )
{
    TaskGuard taskGuard(getTask(tmIds.getTaskId()));
    Task* task = taskGuard.get();
    if (!task) {
        if (!internal) smsc_log_warn(log_, "Response: Unable to locate task '%d' for sequence number=%d" ,
                                     tmIds.taskId, rd.seqNum);
        return false;
    }

    const TaskInfo& info = task->getInfo();

    if (!rd.accepted || internal)
    {
        if (rd.retry && (rd.immediate || (info.retryOnFail && info.retryPolicy.length())))
        {
            time_t nextTime = time(NULL)+(rd.immediate ? 0 :
                                          TaskProcessor::getRetryPolicies().getRetryTime(info.retryPolicy.c_str(),rd.status));

            if ((info.endDate>0 && nextTime >=info.endDate) ||
                (info.validityDate>0 && nextTime>=info.validityDate))
            {
                task->finalizeMessage(tmIds.msgId, EXPIRED, rd.status );
                statistics->incFailed(info.uid);
            } 
            else
            {
                if (!task->retryMessage(tmIds.msgId, nextTime)) {
                    smsc_log_warn(log_, "Message #%llx not found for retry.", tmIds.msgId);
                    statistics->incFailed(info.uid);
                } 
                else if (!rd.immediate) statistics->incRetried(info.uid);
            }
        }
        else
        {
            task->finalizeMessage(tmIds.msgId, FAILED, rd.status );
            statistics->incFailed(info.uid);
        }
        return false;
    }

    if (info.transactionMode) {
        task->finalizeMessage(tmIds.msgId, DELIVERED, rd.status );
        statistics->incDelivered(info.uid);
        return false;
    }
        
    // need receipt
    if ( !receipted ) {
        if (!task->enrouteMessage(tmIds.msgId) ) {
            throw Exception("Message #%llx not found(doEnroute).",tmIds.msgId);
        }
    }
    return true;
}


void TaskProcessor::processMessage(const TaskMsgId& tmIds,const ResponseData& rd)
{
    TaskGuard taskGuard = getTask(tmIds.taskId);
    Task* task = taskGuard.get();
    if (!task) {
        throw Exception("processMessage(): Unable to locate task '%d' for smscMsgId='%s'",
                        tmIds.taskId, rd.msgId.c_str());
    }

    // __require__(task);

    if (rd.accepted)
    {
        task->finalizeMessage(tmIds.msgId, DELIVERED, rd.status );
        statistics->incDelivered(task->getInfo().uid);
    }
    else
    {
        TaskInfo info = task->getInfo();
        if (rd.retry && (rd.immediate || (info.retryOnFail && info.retryPolicy.length())))
        {
            time_t nextTime = time(NULL)+(rd.immediate ? 0:retryPlcs.getRetryTime(info.retryPolicy.c_str(),rd.status));

            if ((info.endDate>0 && nextTime >=info.endDate) ||
                (info.validityDate>0 && nextTime>=info.validityDate))
            {
                task->finalizeMessage(tmIds.msgId, EXPIRED, rd.status);
                statistics->incFailed(info.uid);
            } 
            else
            {
                if (!task->retryMessage(tmIds.msgId, nextTime)) {
                    smsc_log_warn(log_, "Message #%lld not found for retry.", tmIds.msgId);
                    statistics->incFailed(info.uid);
                } 
                else if (!rd.immediate) statistics->incRetried(info.uid);
            }
        }
        else
        { 
            task->finalizeMessage(tmIds.msgId, FAILED, rd.status );
            statistics->incFailed(info.uid);
        }
    }
}

/* ------------------------ Admin interface implementation ------------------------ */ 

void TaskProcessor::reloadSmscAndRegions()
{
    MutexGuard msGuard(messageSenderLock);
    if ( !messageSender ) return;
    Manager::reinit();
    Manager& config = Manager::getInstance();
    messageSender->reloadSmscAndRegions( config );
}


void TaskProcessor::addTask(uint32_t taskId)
{

    Task* task = 0; bool delivery = false;
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%u", taskId);
        ConfigView taskConfig(config, taskSection);
        
        try { delivery = taskConfig.getBool("delivery"); }
        catch (ConfigException& ce) { delivery = false; }


        DataSource* taskDs = 0;
        if (!delivery)
        {
            const char* ds_id = taskConfig.getString("dsId");
            if (!ds_id || ds_id[0] == '\0')
                throw ConfigException("DataSource id for task '%d' empty or wasn't specified",
                                      taskId);
            taskDs = provider.getDataSource(ds_id);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task '%d'", 
                                      ds_id, taskId);
        }

        std::string location=storeLocation;
        char buf[32];
        sprintf(buf,"%u/",taskId);
        location+=buf;

        if(!buf::File::Exists(location.c_str()))
        {
          smsc_log_info(log_,"creating new dir:'%s' for taskId=%u",location.c_str(),taskId);
          buf::File::MkDir(location.c_str());
        }

        task = new Task(&taskConfig, taskId, location, taskDs, finalStateSaver_.get() );
        if (!task) 
            throw Exception("New task create failed");
        if (!addTask(task))
            throw ConfigException("Failed to add task. Task with id '%u' already registered.",
                                  taskId);
       
    } catch (std::exception& exc) {
        if (task && !delivery) task->destroy();
        smsc_log_error(log_, "Failed to add task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) {
        if (task && !delivery) task->destroy();
        smsc_log_error(log_, "Failed to add task '%d'. Cause is unknown", taskId);
        throw Exception("Cause is unknown");
    }
}
void TaskProcessor::removeTask(uint32_t taskId)
{
    try
    {
        scheduler.removeTask(taskId);
        if (!delTask(taskId)) throw Exception("Task not found.");  
        if (statistics) statistics->delStatistics(taskId);
    
    } catch (std::exception& exc) {
        smsc_log_error(log_, "Failed to remove task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) {
        smsc_log_error(log_, "Failed to remove task '%d'. Cause is unknown", taskId);
        throw Exception("Cause is unknown");
    }
}
void TaskProcessor::changeTask(uint32_t taskId)
{
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%d", taskId);
        ConfigView taskConfig(config, taskSection);
        TaskGuard tg(getTask(taskId));
        Task* task = tg.get();
        task->update(&taskConfig);

        /*
        try { delivery = taskConfig.getBool("delivery"); }
        catch (ConfigException& ce) { delivery = false; }

        DataSource* taskDs = 0;
        if (!delivery)
        {
          const char* ds_id = taskConfig.getString("dsId");
          if (!ds_id || ds_id[0] == '\0')
              throw ConfigException("DataSource id for task '%d' empty or wasn't specified",
                                    taskId);
          taskDs = provider.getDataSource(ds_id);
          if (!taskDs)
              throw ConfigException("Failed to obtail DataSource driver '%s' for task '%d'", 
                                    ds_id, taskId);
        }
        
        if (!remTask(taskId))
        {
          smsc_log_warn(log_, "Failed to change task. Task with id '%d' wasn't registered.", taskId);
        }
        std::string location=storeLocation;
        char buf[32];
        sprintf(buf,"%u/",taskId);
        location+=buf;
        if(!buf::File::Exists(location.c_str()))
        {
          smsc_log_info(log_,"creating new dir:'%s' for taskId=%u",location.c_str(),taskId);
          buf::File::MkDir(location.c_str());
        }
        task = new Task(&taskConfig, taskId, location, taskDs);
        if (!task) 
        {
          throw Exception("New task create failed");
        }
        if (!putTask(task))
        {
          smsc_log_warn(log_, "Failed to change task with id '%d'. Task was re-registered", taskId);
          if (!delivery) task->destroy();
        }
        */
    } catch (std::exception& exc)
    {
        //if (task && !delivery) task->destroy();
        smsc_log_error(log_, "Failed to change task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) 
    {
        //if (task && !delivery) task->destroy();
        smsc_log_error(log_, "Failed to change task '%d'. Cause is unknown", taskId);
        throw Exception("Cause is unknown");
    }
}
bool TaskProcessor::startTask(uint32_t taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false;
    if (!task->canGenerateMessages()) return true;
    return (task->isInGeneration()) ? true:invokeBeginGeneration(task);
}
bool TaskProcessor::stopTask(uint32_t taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    if (!task->canGenerateMessages()) return true;
    return (task->isInGeneration()) ? invokeEndGeneration(task):true;
}
Array<std::string> TaskProcessor::getGeneratingTasks()
{
    MutexGuard guard(tasksLock);
    
    Array<std::string> generatingTasks;

    int key = 0;
    Task* task = 0;
    IntHash<Task*>::Iterator it=tasks.First();

    while (it.Next(key, task))
    {
      if (task && task->isInGeneration()) 
      {
        generatingTasks.Push(task->getIdStr());
      }
    }

    return generatingTasks;
}
Array<std::string> TaskProcessor::getProcessingTasks()
{
    MutexGuard guard(tasksLock);
    
    Array<std::string> processingTasks;

    int key = 0;
    Task* task = 0;
    IntHash<Task*>::Iterator it=tasks.First();
    while (it.Next(key, task))
    {
      if (task && task->isInProcess()) 
      {
        processingTasks.Push(task->getIdStr());
      }
    }

    return processingTasks;
}
bool TaskProcessor::isTaskEnabled(uint32_t taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    return (task && task->isEnabled());
}
bool TaskProcessor::setTaskEnabled(uint32_t taskId, bool enabled)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    task->setEnabled(enabled);
    if (enabled) awake.Signal(); 
    return true;
}
void TaskProcessor::addSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') throw Exception("Schedule id is empty");

    Schedule* schedule = 0;
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char scheduleSection[1024];
        sprintf(scheduleSection, "InfoSme.Schedules.%s", schedule_id);
        ConfigView scheduleConfig(config, scheduleSection);
        
        schedule = Schedule::create(&scheduleConfig, scheduleId);
        if (!schedule) 
            throw Exception("New schedule create failed");
        if (!scheduler.addSchedule(schedule))
            throw Exception("Schedule with id '%s' was alredy registered");
    
    } catch (Exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed to add schedule '%s'. Details: %s", schedule_id, exc.what());
        throw;
    } catch (std::exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed to add schedule '%s'. Details: %s", schedule_id, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed add schedule '%s'. Cause is unknown", schedule_id);
        throw Exception("Cause is unknown");
    }
}
void TaskProcessor::removeSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') throw Exception("Schedule id is empty");
    if (!scheduler.removeSchedule(scheduleId))
        throw Exception("Schedule with id '%s' not found");
}
void TaskProcessor::changeSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') throw Exception("Schedule id is empty");

    Schedule* schedule = 0;
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char scheduleSection[1024];
        sprintf(scheduleSection, "InfoSme.Schedules.%s", schedule_id);
        ConfigView scheduleConfig(config, scheduleSection);
        
        schedule = Schedule::create(&scheduleConfig, scheduleId);
        if (!schedule) 
            throw Exception("New schedule create failed");
        if (!scheduler.changeSchedule(scheduleId, schedule)) {
            delete schedule;
            smsc_log_warn(log_, "Failed to change schedule with id '%s'. Schedule was re-registered", schedule_id);
        }
    
    } catch (Exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed to change schedule '%s'. Details: %s", schedule_id, exc.what());
        throw;
    } catch (std::exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed to change schedule '%s'. Details: %s", schedule_id, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        if (schedule) delete schedule;
        smsc_log_error(log_, "Failed change schedule '%s'. Cause is unknown", schedule_id);
        throw Exception("Cause is unknown");
    }
}

void TaskProcessor::addDeliveryMessages(uint32_t taskId,
                                        uint8_t msgState,
                                        const std::string& abonentAddress,
                                        time_t messageDate,
                                        const std::string& msg,
                                        const std::string& userData )
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::addDeliveryMessages::: can't get task by taskId='%d'", taskId);
  task->insertDeliveryMessage(msgState, abonentAddress, messageDate, msg, userData);
  statistics->incGenerated(taskId);
}

void TaskProcessor::changeDeliveryMessageInfoByRecordId(uint32_t taskId,
                                                        uint8_t msgState,
                                                        time_t unixTime,
                                                        const std::string& recordId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::changeDeliveryMessageInfoByRecordId::: can't get task by taskId='%d'", taskId);
  uint64_t newMsgId;
  if(task->changeDeliveryMessageInfoByRecordId(msgState, unixTime, recordId,newMsgId))
  {
    smsc_log_debug(log_,"msgId=#%s changed to #%llx",recordId.c_str(),newMsgId);
  }else
  {
    smsc_log_debug(log_,"changeDeliveryMessageInfoByRecordId failed msgId=#%s",recordId.c_str());
  }
}

void TaskProcessor::changeDeliveryMessageInfoByCompositCriterion(uint32_t taskId,
                                                                 uint8_t msgState,
                                                                 time_t unixTime,
                                                                 const InfoSme_T_SearchCriterion& searchCrit)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::changeDeliveryMessageInfoByCompositCriterion::: can't get task by taskId='%d'", taskId);
  task->changeDeliveryMessageInfoByCompositCriterion(msgState, unixTime, searchCrit);
}

void TaskProcessor::deleteDeliveryMessageByRecordId(uint32_t taskId,
                                                    const std::string& recordId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::deleteDeliveryMessageByRecordId::: can't get task by taskId='%d'", taskId);
  task->deleteDeliveryMessageByRecordId(recordId);
}

void TaskProcessor::deleteDeliveryMessagesByCompositCriterion(uint32_t taskId,
                                                              const InfoSme_T_SearchCriterion& searchCrit)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::deleteDeliveryMessagesByCompositCriterion::: can't get task by taskId='%d'", taskId);
  task->deleteDeliveryMessagesByCompositCriterion(searchCrit);
}

extern const char* INSERT_TASK_STAT_STATE_SQL;
extern const char* INSERT_TASK_STAT_STATE_ID;


/*
void TaskProcessor::insertRecordIntoTasksStat(uint32_t taskId,
                                              uint32_t period,
                                              uint32_t generated,
                                              uint32_t delivered,
                                              uint32_t retried,
                                              uint32_t failed)
{
  std::auto_ptr<Statement> statement(dsIntConnection->createStatement(INSERT_TASK_STAT_STATE_SQL));
  if (!statement.get())
    throw Exception("Failed to obtain statement for statistics update");

  char buf[32];
  sprintf(buf,"%u",taskId);

  statement->setString(1, buf);
  statement->setUint32(2, period);
  statement->setUint32(3, generated);
  statement->setUint32(4, delivered);
  statement->setUint32(5, retried);
  statement->setUint32(6, failed);

  statement->executeUpdate();
  !!TODO!! STATS! :(
  
}
  */

Array<std::string> TaskProcessor::getTaskMessages(const uint32_t taskId,
                                                  const InfoSme_T_SearchCriterion& searchCrit)
{
  try {
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) throw Exception("TaskProcessor::getTaskMessages::: can't get task by taskId='%d'", taskId);
    return task->selectDeliveryMessagesByCompositCriterion(searchCrit);
  } catch (std::exception& ex) {
    smsc_log_error(log_, "TaskProcessor::getTaskMessages::: catch exception=[%s]", ex.what());
    throw;
  } catch (...) {
    smsc_log_error(log_, "TaskProcessor::getTaskMessages::: catch unexpected exception");
    throw;
  }
}

bool TaskProcessor::doesMessageConformToCriterion(ResultSet* rs, const InfoSme_Tasks_Stat_SearchCriterion& searchCrit)
{
  if ( searchCrit.isSetTaskId() && 
       searchCrit.getTaskId() != rs->getString(1) )
    return false;

  if ( searchCrit.isSetStartPeriod() && 
       searchCrit.getStartPeriod() > rs->getUint32(2) )
    return false;

  if ( searchCrit.isSetEndPeriod() && 
       searchCrit.getEndPeriod() < rs->getUint32(2) )
    return false;

  return true;
}

const char* DO_FULL_TABLESCAN_TASKS_STAT_STATEMENT_ID = "FULL_TABLESCAN_TASKS_STAT_STATEMENT_ID";
const char* DO_FULL_TABLESCAN_TASKS_STAT_STATEMENT_SQL = "FULL_TABLE_SCAN FROM INFOSME_TASKS_STAT";

struct TaskStatDescription {
  TaskStatDescription(const std::string& anTaskId="", uint32_t aPeriod=0, uint32_t aGenerated=0,
                      uint32_t aDelivered=0, uint32_t aRetried=0, uint32_t aFailed=0)
    : taskId(anTaskId), period(aPeriod), generated(aGenerated), delivered(aDelivered), retried(aRetried), failed(aFailed) {}

  TaskStatDescription& operator+(const TaskStatDescription& rhs) {
    if ( period == rhs.period ) {
      generated += rhs.generated;
      delivered += rhs.delivered;
      retried += rhs.retried;
      failed += rhs.failed;
    }
    return *this;
  }
  std::string taskId;
  uint32_t period;
  uint32_t generated;
  uint32_t delivered;
  uint32_t retried;
  uint32_t failed;
};

static bool orderBinaryPredicate(const TaskStatDescription& lhs,
                                 const TaskStatDescription& rhs)
{
  if ( lhs.period < rhs.period ) return true;
  else return false;
}

/*
Array<std::string> TaskProcessor::getTasksStatistic(const InfoSme_Tasks_Stat_SearchCriterion& searchCrit)
{
  std::auto_ptr<Statement> selectMessage(dsIntConnection->createStatement(DO_FULL_TABLESCAN_TASKS_STAT_STATEMENT_SQL));
  if (!selectMessage.get())
    throw Exception("getTasksStatistic(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());

  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");

  typedef std::list<TaskStatDescription> TasksStatList_t;
  TasksStatList_t statisticsList;

  int fetched = 0;
  while (rs->fetchNext()) {
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      smsc_log_debug(log_, "TaskProcessor::getTasksStatistic::: add statistic [%s,%d,%d,%d,%d,%d] into statisticsList", rs->getString(1), rs->getUint32(2), rs->getUint32(3), rs->getUint32(4), rs->getUint32(5), rs->getUint32(6));
      statisticsList.push_back(TaskStatDescription(rs->getString(1),
                                                   rs->getUint32(2),
                                                   rs->getUint32(3),
                                                   rs->getUint32(4),
                                                   rs->getUint32(5),
                                                   rs->getUint32(6)));
    }
  }

  statisticsList.sort(orderBinaryPredicate);

  TasksStatList_t accumulatedStatisticsList;

  TasksStatList_t::iterator iter = statisticsList.begin();

  if ( iter != statisticsList.end() ) {
    TaskStatDescription prevElement(*iter);
    TaskStatDescription taskStatResult(*iter);
    while ( ++iter != statisticsList.end() ) {
      if ( iter->period != prevElement.period ) {
        accumulatedStatisticsList.push_back(taskStatResult);
        taskStatResult = *iter;
      } else
        taskStatResult = taskStatResult + *iter;
      prevElement = *iter;
    }
    accumulatedStatisticsList.push_back(taskStatResult);
  }

  Array<std::string> tasksStat;
  for(TasksStatList_t::iterator iter=accumulatedStatisticsList.begin();
      iter!=accumulatedStatisticsList.end(); ++iter) {
    std::ostringstream statisticBuf;
    statisticBuf << iter->period << "|"
                 << iter->generated << "|"
                 << iter->delivered << "|"
                 << iter->retried << "|"
                 << iter->failed;
    tasksStat.Push(statisticBuf.str());
  }

  return tasksStat;
}*/

void
TaskProcessor::endDeliveryMessagesGeneration(uint32_t taskId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::endDeliveryMessagesGeneration::: can't get task by taskId='%d'", taskId);
  task->endDeliveryMessagesGeneration();
}

void
TaskProcessor::changeDeliveryTextMessageByCompositCriterion(uint32_t taskId,
                                                            const std::string& textMsg,
                                                            const InfoSme_T_SearchCriterion& searchCrit)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::changeDeliveryTextMessageByCompositCriterion::: can't get task by taskId='%d'", taskId);
  task->changeDeliveryTextMessageByCompositCriterion(textMsg, searchCrit);
}

TaskInfo TaskProcessor::getTaskInfo(uint32_t taskId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  return task?task->getInfo():TaskInfo();
}
  
void TaskProcessor::applyRetryPolicies()
{
  Manager::reinit();
  Manager& config = Manager::getInstance();
  ConfigView retryPlcCfg(config,"RetryPolicies");
  retryPlcs.Load(&retryPlcCfg);
}

uint32_t TaskProcessor::sendSms(const std::string& src,const std::string& dst,const std::string& msg,bool flash)
{
    return messageSender->sendSms(src,dst,msg,flash);
}

}
}
