#include "TaskProcessor.h"
#include "InfoSmeMessageSender.h"
#include "FinalStateSaver.h"
#include "SmscConnector.h"
#include "MsecTime.h"
#include "util/config/ConfString.h"
#include <exception>
#include <list>
#include <cstdlib>
#include <sstream>
#include <vector>

#ident "@(#)$Id$"

extern bool isMSISDNAddress(const char* string);

namespace {

struct Drand48adapter
{
public:
    Drand48adapter() {
        srand48(time(0));
    }
    template <typename T> T operator () ( T tmax ) {
        return T( tmax * drand48() );
    }
};

Drand48adapter drand48adapter;

}


namespace smsc {
namespace infosme {

using namespace smsc::util::config;

RetryPolicies TaskProcessor::retryPlcs;

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor() :
InfoSmeAdmin(), Thread(),
log_(Logger::getInstance("is2.proc")), 
dispatcher_(5000),
provider(0),
bStarted(false),
bNeedExit(false),
messageSender(new InfoSmeMessageSender(dispatcher_)),
responseWaitTime(0),
receiptWaitTime(0),
mappingRollTime(0),
mappingMaxChanges(0),
dsStatConnection(0),
statistics(0), protocolId(0), svcType(0),
unrespondedMessagesMax(1)
// unrespondedMessagesSleep(10)
{
    smsc_log_info(log_, "ctor...");
}


void TaskProcessor::init( ConfigView* config )
{
    smsc_log_info(log_, "init ...");
    MutexGuard mg(startLock);

    storeLocation=ConfString(config->getString("storeLocation")).str();
    if(storeLocation.length())
    {
      if(*storeLocation.rbegin()!='/')
      {
        storeLocation+='/';
      }
    }

    address = ConfString(config->getString("Address")).str();
    if ( address.empty() || !isMSISDNAddress(address.c_str()))
        throw ConfigException("Address string '%s' is invalid", address.empty() ? "-" : address.c_str());
    
    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = ConfString(config->getString("SvcType")).str(); }
    catch(ConfigException& exc) { svcType.clear(); };
    
    responseWaitTime = parseTime(ConfString(config->getString("responceWaitTime")).c_str());
    if (responseWaitTime <= 0) 
        throw ConfigException("Invalid value for 'responceWaitTime' parameter.");
    receiptWaitTime = parseTime(ConfString(config->getString("receiptWaitTime")).c_str());
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
    
    std::auto_ptr<ConfigView> retryPlcCfg(config->getSubConfig("RetryPolicies"));
    retryPlcs.Load(retryPlcCfg.get());

    std::auto_ptr<ConfigView> tasksThreadPoolCfgGuard(config->getSubConfig("TasksThreadPool"));
    taskManager.init(tasksThreadPoolCfgGuard.get());   // loads up thread pool for tasks
    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("EventsThreadPool"));
    eventManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    try {
        std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
        if ( !provider ) provider = new DataProvider;
        provider->init(providerCfgGuard.get());
    } catch (...) {
        smsc_log_warn(log_,"DataProvider section is not found");
    }
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("systemDataSource"));

    mappingRollTime = config->getInt("mappingRollTime");
    mappingMaxChanges = config->getInt("mappingMaxChanges");
    
    messageSender->init( *this, config );
    finalStateSaver_.reset( new FinalStateSaver(storeLocation) );

    statistics = new StatisticsManager(ConfString(config->getString("statStoreLocation")).c_str(),this);
    if (statistics) statistics->Start();

    smsc_log_info(log_, "Loading tasks ...");
    std::auto_ptr< ConfigView > taskConfig;
    std::auto_ptr< std::set< std::string> > taskNames;
    try {
        taskConfig.reset(config->getSubConfig("Tasks"));
        taskNames.reset( taskConfig->getShortSectionNames() );
    } catch ( ConfigException& ) {
        smsc_log_warn(log_,"problem reading section 'Tasks'");
    }
    if ( !taskNames.get() ) { taskNames.reset(new std::set<std::string>()); }

    // adding tasks that are in storelocation
    std::vector< std::string > entries;
    entries.reserve(200);
    smsc::core::buffers::File::ReadDir( storeLocation.c_str(),
                                        entries );
    for ( std::vector< std::string >::const_iterator i = entries.begin();
          i != entries.end();
          ++i ) {
        if ( atoi(i->c_str()) != 0 ) {
            if ( taskNames->insert(*i).second ) {
                smsc_log_debug(log_,"adding task name '%s' to the list",i->c_str());
            }
        }
    }
    // finally read all tasks
    for ( std::set< std::string >::const_iterator i = taskNames->begin();
          i != taskNames->end(); ++i ) {

        try {
            unsigned id = Task::stringToTaskId(i->c_str());
            initTask( id, taskConfig.get() );
        } catch ( ConfigException& e ) {
            smsc_log_warn(log_,"cannot load task %s: %s",i->c_str(),e.what());
        }
    }

    smsc_log_info(log_, "Tasks loaded.");

    smsc_log_info(log_, "Loading task schedules ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Schedules"));
    scheduler.init(this,schedulerCfgGuard.get());
    smsc_log_info(log_, "Task schedules loaded.");
    
    scheduler.Start();
    smsc_log_info(log_, "init finished");
}


TaskProcessor::~TaskProcessor()
{
    smsc_log_debug(log_,"dtor");
    // jstore.Stop();
  scheduler.Stop();
  this->Stop();
  taskManager.Stop();
  eventManager.Stop();
  if (statistics) statistics->Stop();
  
  {
      MutexGuard guard(tasksLock);
      int key = 0;
      TaskGuard* task;
      TaskHash::Iterator it=tasks.First();
      while (it.Next(key, task))
          if (task->get()) (*task)->shutdown();
      tasks.Empty();
  }

  taskManager.shutdown();
  eventManager.shutdown();

    if (statistics) delete statistics;
    if (provider) delete provider;
    if (messageSender) delete messageSender;
    smsc_log_debug(log_,"dtor finished");
}


TaskGuard TaskProcessor::getTask( uint32_t taskId, bool remove )
{
    MutexGuard guard(tasksLock);
    TaskGuard* ptr = tasks.GetPtr(taskId);
    if (!ptr || (*ptr)->isFinalizing() ) return TaskGuard();
    if (!remove) { return *ptr; }
    // remove
    TaskGuard ret(*ptr);
    tasks.Delete(taskId);
    dispatcher_.delTask(*ret.get());
    scheduler.removeTask(ret->getId());
    if ( statistics ) statistics->delStatistics(ret->getId());
    return ret;
}

void TaskProcessor::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    
    int key = 0; 
    TaskGuard task = 0;
    TaskHash::Iterator it=tasks.First();
    while (it.Next(key,task))
        if (task.get()) task->resetWaiting();
}

void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    if (!bStarted)
    {
        smsc_log_info(log_, "Starting ...");

        resetWaitingTasks();
        bNeedExit = false;
        notified_ = false;
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
        while ( bStarted ) {
            startLock.notifyAll();
            startLock.wait(100);
        }
        // bStarted = false;
        smsc_log_info(log_, "Stoped.");
    }
}
int TaskProcessor::Execute()
{
    const msectime_type startTime = currentTimeMillis();
    smsc_log_info(log_,"execute started");

    msectime_type currentTime = startTime;
    msectime_type movingStart = startTime;
    msectime_type nextWakeTime = startTime;
    msectime_type lastNotifyTime = startTime;
    msectime_type lastStatTime = startTime;

    bool wasNotified = false;
    try {
        while ( true ) {

            currentTime = currentTimeMillis();

            // 1. sleeping until some interesting events are there
            {
                if ( bNeedExit ) break;
                int waitTime = int(nextWakeTime - currentTime);
                if ( waitTime > 0 ) {
                    if ( waitTime < 10 ) waitTime = 10;
                    MutexGuard mg(startLock);
                    wasNotified = notified_;
                    notified_ = false;
                    if ( !wasNotified ) {
                        smsc_log_debug(log_,"want to sleep %u ms",waitTime);
                        startLock.wait( waitTime );
                        continue;
                    }
                }
            }

            unsigned deltaTime = unsigned(currentTime - movingStart);

            // 2. flipping start time
            if ( deltaTime > 1000000 ) {
                movingStart += deltaTime;
                deltaTime = 0;
                smsc_log_info(log_,"making a flip for a startTime");
            }

            smsc_log_debug(log_,"new pass at %u, notified=%u",deltaTime,wasNotified);

            // 3. dumping statistics
            if ( currentTime - lastStatTime > 60000 ) { // one minute
                MutexGuard mg(tasksLock);
                int key;
                TaskGuard* tg;
                unsigned activeOpenMessages = 0, totalOpenMessages = 0;
                unsigned activeTasksCount = 0;
                for ( TaskHash::Iterator it(tasks); it.Next(key,guard); ) {
                    if (!tg) continue;
                    Task* task = tg->get();
                    if (!task) continue;
                    unsigned openMessages;
                    task->collectStatistics(openMessages);
                    totalOpenMessages += openMessages;
                    if (task->isActive()) {
                        ++activeTasksCount;
                        activeOpenMessages += openMessages;
                    }
                    smsc_log_info(log_,"task %u/'%s' msgs=%u",
                                  key,task->getName().c_str(),openMessages);
                }
                smsc_log_info(log_,"Stats follows: tasks active/total=%u/%u msgs=%u/%u",
                              activeTasksCount(), tasks.Count(),
                              activeOpenMessages,totalOpenMessages);
            }

            // 4. processing notification
            MutexGuard mg(tasksLock);
            while ( wasNotified || (currentTime - lastNotifyTime) > 30000 ) {
                lastNotifyTime = currentTime;
                wasNotified = false;
                smsc_log_debug(log_,"notified=%u or time passed",wasNotified);
                checkTaskActivity();
                dispatcher_.removeInactiveTasks();
            }

            // 5. processing
            nextWakeTime = currentTime + messageSender->send(deltaTime,dispatcher_.sleepTime());

            // 6. process waiting events (timeouted responses)
            messageSender->processWaitingEvents(time(0));
        }

    } catch ( std::exception& e ) {
        smsc_log_error(log_,"exc in execute, stopping: %s", e.what());
    }
    MutexGuard mg(startLock);
    bStarted = false;
    startLock.notifyAll();
    smsc_log_info(log_,"execute finished");
    return 0;
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

/* ------------------------ ServicesForTask interface implementation ------------------------ */ 

int TaskProcessor::findRegionByAddress( const char* addr )
{
    if ( !messageSender ) return -2; // failure
    return messageSender->findRegionByAddress( addr );
}


void TaskProcessor::saveFinalState( time_t now,
                                    const TaskInfo& info,
                                    const Message&  msg,
                                    uint8_t         state,
                                    int             smppStatus,
                                    bool            noMoreMessages )
{
    if ( finalStateSaver_.get() )
        finalStateSaver_->save(now,info,msg,state,smppStatus,noMoreMessages);
}


/* ------------------------ Admin interface implementation ------------------------ */ 

void TaskProcessor::reloadSmscAndRegions()
{
    smsc_log_info(log_,"reloadSmscAndRegions invoked");
    MutexGuard msGuard(tasksLock);
    if ( !messageSender ) return;
    Manager::reinit();
    ConfigView config(Manager::getInstance(),"InfoSme");
    messageSender->init( *this, &config );
}


void TaskProcessor::initTask( uint32_t id, ConfigView* taskConfig )
{
    smsc_log_info(log_,"loading task %u...", id );
    char taskId[30];
    sprintf(taskId,"%u",id);

    MutexGuard mg(tasksLock);

    std::auto_ptr< Config > separateConfig;
    std::auto_ptr< ConfigView > separateView;
    const std::string location = storeLocation + taskId;
    if ( smsc::core::buffers::File::Exists(location.c_str()) ) {
        const std::string fname = location + "/config.xml";
        if ( smsc::core::buffers::File::Exists(fname.c_str()) ) {
            separateConfig.reset(Config::createFromFile(fname.c_str()));
            separateView.reset(new ConfigView(*separateConfig.get()));
            smsc_log_debug(log_,"task %u is using a separate config '%s'",id,fname.c_str());
            taskConfig = separateView.get();
        }
    }
    if ( ! separateView.get() ) {
        // not loaded, using default config
        if ( !taskConfig ) {
            // not passed in, reading from Manager
            try {
                Manager::reinit();
                Manager& mgr(Manager::getInstance());
                separateView.reset( new ConfigView(mgr,"InfoSme") );
                separateView.reset( separateView->getSubConfig("Tasks") );
                taskConfig = separateView.get();
            } catch (...) {
                throw ConfigException("tasks config is not passed in, and cannot be retrieved");
            }
        }
        separateView.reset( taskConfig->getSubConfig(taskId) );
        smsc_log_debug(log_,"task %u is using an embedded config",id);
        taskConfig = separateView.get();
    }

    TaskInfo info(id);
    info.init(taskConfig);

    TaskGuard* ptr( tasks.GetPtr(id) );
    if ( ptr && ptr->get() ) {

        (*ptr)->setInfo(info);

    } else {

        smsc::db::DataSource* taskDs = 0;
        if ( !info.delivery ) {
            const char* dsId = taskConfig->getString("dsId");
            if (!dsId || dsId[0] == '\0')
                throw ConfigException("DataSource id for task %u is empty or wasn't specified",
                                      id);
            if ( !provider ) {
                throw ConfigException("DataSourceProvider is not provided");
            }
            taskDs = provider->getDataSource(dsId);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task %u",
                                      dsId, id);
        }
        if ( ptr ) { tasks.Delete(id); }
        TaskGuard& guard = tasks.Insert(id,TaskGuard().create(id,location,info,taskDs,this));
        if ( guard.get() && guard->isActive() ) {
            dispatcher_.addTask( *guard.get() );
        }
    }
    notified_ = true;
    startLock.notifyAll();
    smsc_log_info(log_,"task %u is loaded",id);
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
    TaskGuard task = 0;
    TaskHash::Iterator it=tasks.First();

    while (it.Next(key, task))
    {
        if (task.get() && task->isInGeneration())
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
    TaskGuard task = 0;
    TaskHash::Iterator it=tasks.First();
    while (it.Next(key, task))
    {
        if (task.get() && task->isInProcess())
      {
        processingTasks.Push(task->getIdStr());
      }
    }

    return processingTasks;
}

bool TaskProcessor::isTaskEnabled(uint32_t taskId)
{
    TaskGuard task = getTask(taskId);
    return (task.get() && task->isEnabled());
}

bool TaskProcessor::setTaskEnabled(uint32_t taskId, bool enabled)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    task->setEnabled(enabled);
    awakeSignal();
    return true;
}

void TaskProcessor::addSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') throw Exception("Schedule id is empty");

    MutexGuard mg(tasksLock);
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
    awakeSignal();
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
    awakeSignal();
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


void
TaskProcessor::endDeliveryMessagesGeneration(uint32_t taskId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::endDeliveryMessagesGeneration::: can't get task by taskId='%d'", taskId);
  task->endDeliveryMessagesGeneration();
    awakeSignal();
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
    awakeSignal();
}

  
void TaskProcessor::applyRetryPolicies()
{
    MutexGuard mg(tasksLock);
  Manager::reinit();
  Manager& config = Manager::getInstance();
  ConfigView retryPlcCfg(config,"RetryPolicies");
  retryPlcs.Load(&retryPlcCfg);
}

uint32_t TaskProcessor::sendSms(const std::string& src,const std::string& dst,const std::string& msg,bool flash)
{
    if (!messageSender) throw smsc::util::Exception("message sender is not set");
    return messageSender->sendSms(src,dst,msg,flash);
}


void TaskProcessor::checkTaskActivity()
{
    time_t now = time(NULL);
    tm locTm;
    localtime_r(&now,&locTm);
    smsc_log_debug(log_,"check task activity at %ld %04u-%02u-%02u+%02u:%02u:%02u",
                   long(now), 1900+locTm.tm_year, locTm.tm_mon+1, locTm.tm_mday,
                   locTm.tm_hour, locTm.tm_min, locTm.tm_sec );
    int key;
    TaskGuard* ptr;
    for ( TaskHash::Iterator i(tasks); i.Next(key,ptr); ) {
        if (ptr && ptr->get()) {
            (*ptr)->checkActivity(now,locTm);
            if ( (*ptr)->isActive() ) {
                dispatcher_.addTask(*(ptr->get()));
            }
        }
    }
}

}
}
