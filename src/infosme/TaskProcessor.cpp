#include "TaskProcessor.h"
#include "MessageSender.h"
#include "FinalStateSaver.h"
#include "SmscConnector.h"
#include <exception>
#include <list>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <util/timeslotcounter.hpp>
#include "util/PtrDestroy.h"
#include "util/config/ConfString.h"

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

TaskProcessor::TaskProcessor(ConfigView* config) :
TaskProcessorAdapter(), InfoSmeAdmin(), Thread(),
log_(Logger::getInstance("smsc.infosme.TaskProcessor")), 
#ifdef INTHASH_USAGE_CHECKING
tasks(SMSCFILELINE),
#endif
bStarted(false),
bNeedExit(false),
messageSender(0), 
responseWaitTime(0),
receiptWaitTime(0),
mappingRollTime(0),
mappingMaxChanges(0),
dsStatConnection(0),
statistics(0), protocolId(0),
unrespondedMessagesMax(1),
maxMessageChunkSize_(0)
// unrespondedMessagesSleep(10)
{
    smsc_log_info(log_, "Loading ...");

    address = ConfString(config->getString("Address")).str();
    if (address.empty() || !isMSISDNAddress(address.c_str()))
        throw ConfigException("Address string '%s' is invalid", address.empty() ? "-" : address.c_str() );
    
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

    {
        const char* pn = "maxMessageChunkSize";
        try {
            maxMessageChunkSize_ = config->getInt(pn);
        } catch (...) {
            maxMessageChunkSize_ = 0;
            smsc_log_warn(log_,"Parameter '%s' is invalid. Using default value %d",pn,maxMessageChunkSize_);
        }
        if ( maxMessageChunkSize_ != 0 ) {
            int newval = maxMessageChunkSize_;
            if ( newval < 133 ) {
                newval = 133;
            } else if ( newval > 160 ) {
                newval = 160;
            }
            if (newval != maxMessageChunkSize_) {
                smsc_log_warn(log_,"Parameter '%s' value %d adjusted to %d",pn,maxMessageChunkSize_,newval);
                maxMessageChunkSize_ = newval;
            }
        }
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


    storeLocation=ConfString(config->getString("storeLocation")).str();
    if(storeLocation.length())
    {
      if(*storeLocation.rbegin()!='/')
      {
        storeLocation+='/';
      }
    }

    finalStateSaver_.reset( new FinalStateSaver(storeLocation) );

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
            Task* task = initTask( id, taskConfig.get() ); //TODO implement init task
            if (task && !putTask(task)) {
                task->finalize();
                throw ConfigException("Failed to add task. Task with id '%s' already registered.", id);
            }
        } catch ( ConfigException& e ) {
            smsc_log_warn(log_,"cannot load task %s: %s",i->c_str(),e.what());
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
    
    statistics = new StatisticsManager(ConfString(config->getString("statStoreLocation")).c_str(),this);
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
  if (statistics) statistics->Stop();
  
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

Task* TaskProcessor::initTask( uint32_t id, ConfigView* taskConfig )
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

    //TaskInfo info(id);
    //info.init(taskConfig);

    //TaskGuard* ptr( tasks.GetPtr(id) );
    //if ( ptr && ptr->get() ) {

      //  (*ptr)->setInfo(info);

    if (tasks.Exist(id)) {
        Task* task = tasks.Get(id);
        if (task && !task->isFinalizing()) {
            TaskGuard tg(task);
            tg.get()->update(taskConfig);
            activateFlag = true;
            return tg.get();
        } else {
            TaskGuard tg(0);
            return tg.get();
        }
    } else {

        bool delivery = false;
        try { delivery = taskConfig->getBool("delivery"); 
        } catch (ConfigException& ce) { delivery = false; }

        smsc::db::DataSource* taskDs = 0;

        if ( !delivery ) {
            const char* dsId = taskConfig->getString("dsId");
            if (!dsId || dsId[0] == '\0')
                throw ConfigException("DataSource id for task %u is empty or wasn't specified",
                                      id);
            //if ( !provider ) {
              //  throw ConfigException("DataSourceProvider is not provided");
            //}
            //taskDs = provider->getDataSource(dsId);
            taskDs = provider.getDataSource(dsId);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task %u",
                                      dsId, id);
        }
        Task *task = new Task(taskConfig, id, location, taskDs, finalStateSaver_.get() );
        if (!task) throw Exception("New task create failed");
        return task;
    }
}

TaskGuard TaskProcessor::getTask(uint32_t taskId)
{
    MutexGuard guard(tasksLock);

    if (!tasks.Exist(taskId)) return TaskGuard(0);
    Task* task = tasks.Get(taskId);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}

bool TaskProcessor::putTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    if (tasks.Exist(task->getId())) return false;
    tasks.Insert(task->getId(), task);
    activateFlag = true;
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
    activateFlag = true;
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
        activateFlag = true;
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

void TaskProcessor::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    
    int key = 0; 
    Task* task = 0;
    IntHash<Task*>::Iterator it=tasks.First();
    while (it.Next(key, task))
        if (task) task->resetWaiting();
    activateFlag = true;
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
    std::vector< TaskGuard* > taskGuards;
    taskGuards.reserve(100);
    time_t prevTime = time(NULL);
#ifdef INTHASH_USAGE_CHECKING
    IntHash< Task* > activeTasks(SMSCFILELINE);
#else
    IntHash< Task* > activeTasks;
#endif
    time_t statTime = prevTime;
    while (!bNeedExit)
    {
        time_t currentTime = time(NULL);
        finalStateSaver_->checkRoll(currentTime);

        bool waked;
        int allTaskCount, activeTaskCount;
        {
            MutexGuard guard(tasksLock);
            if ( currentTime - prevTime > 60 ) activateFlag = true;
            waked = activateFlag;
            int key = 0;
            Task* task = 0;
            if ( activateFlag ) {
                activateFlag = 0;
                prevTime = currentTime;
                // adding tasks from tasks to activeTasks
                activeTasks.Empty();
                IntHash<Task*>::Iterator it = tasks.First();
                while ( !bNeedExit && it.Next(key,task) ) {
                    if (task && task->isReady(currentTime,true) ) {
                        activeTasks.Insert(key,task);
                        taskGuards.push_back(new TaskGuard(task));
                    }
                }
            } else {
                // working only with activeTasks
                IntHash<Task*>::Iterator it = activeTasks.First();
                while ( !bNeedExit && it.Next(key,task) ) {
                    if (task && task->isReady(currentTime,true) ) {
                        taskGuards.push_back(new TaskGuard(task));
                    } else {
                        activeTasks.Delete(key);
                    }
                }
            }
            allTaskCount = int(tasks.Count());
            activeTaskCount = int(activeTasks.Count());
        }

        smsc_log_debug(log_,"new pass at %llu, waked=%u all/active/selected=%u/%u/%u",
                       int64_t(currentTime), waked,
                       allTaskCount, activeTaskCount, unsigned(taskGuards.size()));

        {
            // processing selected tasks
            for ( std::vector<TaskGuard*>::const_iterator i = taskGuards.begin();
                  i != taskGuards.end(); ++i ) {
                Task* task = (*i)->get();
                task->currentPriorityFrameCounter = 0;
                task->resetSuspendedRegions();
            }
            std::random_shuffle( taskGuards.begin(), taskGuards.end(),
                                 ::drand48adapter );
        }

        int processed = 0;
        std::vector< uint32_t > becomeInactive;
        becomeInactive.reserve(taskGuards.size());
        while (!taskGuards.empty())
        {
            std::auto_ptr<TaskGuard> taskGuard(taskGuards.back());
            taskGuards.pop_back();
            if (!taskGuard.get()) continue;

            do {
                if (bNeedExit) break;
                Task* task = taskGuard->get();
                if (!task) break;
                if (task->isFinalizing() || !task->isEnabled()) {
                    becomeInactive.push_back(task->getId());
                    break;
                }
                try {
                    const unsigned taskProcessed = processTask(task);
                    if ( ! taskProcessed ) {
                        // no one message in task is processed
                        task->currentPriorityFrameCounter = task->getPriority();
                        if (!task->isEnabled()) task->setEnabled(false);
                        if (!task->isInProcess()) becomeInactive.push_back(task->getId());
                    }
                    processed += taskProcessed;
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"TaskProcessor: task '%d/%s' processing failed: %s",
                                  task->getId(), task->getName().c_str(), e.what() );
                }
            } while ( false );
        }

        if (bNeedExit) break;

        if ( !activateFlag ) {
            for ( std::vector< uint32_t >::const_iterator i = becomeInactive.begin();
                  i != becomeInactive.end(); ++i ) {
                activeTasks.Delete(*i);
            }
        }

        processWaitingEvents(time(NULL)); // ?? or time(NULL)

        if ( currentTime - statTime > 60 ) {
            // printout the statistics
            statTime = currentTime;
            // collect statistics on all active tasks
            MutexGuard mg(tasksLock);
            int key;
            Task* task;
            unsigned activeOpenMessages = 0, totalOpenMessages = 0;
            for ( IntHash<Task*>::Iterator it = tasks.First(); it.Next(key,task); ) {
                if ( !task ) continue;
                // collect the number of messages in the task
                unsigned openMessages;
                task->collectStatistics( openMessages );
                totalOpenMessages += openMessages;
                if ( activeTasks.GetPtr(key) ) {
                    activeOpenMessages += openMessages;
                    smsc_log_info(log_,"task %u/'%s' msgs=%u",
                                  key,task->getName().c_str(),openMessages);
                }
            }
            smsc_log_info(log_,"Stats: tasks active/total=%u/%u msgs=%u/%u",
                          activeTasks.Count(), tasks.Count(),
                          activeOpenMessages,totalOpenMessages);
        }

        if (!bNeedExit && processed <= 0) {
            // smsc_log_debug(log_,"TaskProc: processed=%d waiting %d",processed,switchTimeout);
            awake.Wait(switchTimeout);
        }
    }

    std::for_each(taskGuards.begin(),taskGuards.end(),smsc::util::PtrDestroy());

    exited.Signal();
    return 0;
}


unsigned TaskProcessor::processTask(Task* task)
{
    __require__(task);
    unsigned res = 0;
    // smsc_log_debug(log_, "TaskProc::processTask(%d) taskPrio(cur/tot)=%d/%d",
    // task->getId(), task->currentPriorityFrameCounter, task->getPriority() );
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
    smsc_log_info(log_,"reloadSmscAndRegions invoked");
    MutexGuard msGuard(messageSenderLock);
    if ( !messageSender ) return;
    Manager::reinit();
    Manager& config = Manager::getInstance();
    messageSender->reloadSmscAndRegions( config );
}

void TaskProcessor::changeTask(uint32_t taskId)
{
    try
    {
        /*
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%d", taskId);
        ConfigView taskConfig(config, taskSection);
        */

        //TaskGuard tg(getTask(taskId));
        //Task* task = tg.get();
        //task->update(&taskConfig);
        initTask(taskId, 0);

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

void TaskProcessor::addTask(uint32_t taskId)
{

    Task* task = 0; bool delivery = false;
    try
    {
        /*

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
        */

        //task = new Task(&taskConfig, taskId, location, taskDs, finalStateSaver_.get() );
        //if (!task) 
          //  throw Exception("New task create failed");
        //if (!addTask(task))
            //throw ConfigException("Failed to add task. Task with id '%u' already registered.",
              //                    taskId);
       ConfigView* taskConfig = 0;
       task = initTask(taskId, taskConfig);
       if (taskConfig) {
           try { delivery = taskConfig->getBool("delivery"); 
           } catch (ConfigException& ce) { delivery = false; }
       }
       if (task && !addTask(task)) throw ConfigException("Failed to add task. Task with id '%u' already registered.", taskId);

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
    {
        MutexGuard mg(tasksLock);
        activateFlag = true;
    }
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
