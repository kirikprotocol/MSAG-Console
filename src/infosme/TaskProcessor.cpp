
#include "TaskProcessor.h"
#include <exception>

namespace smsc { namespace infosme 
{

/* ---------------------------- PriorityContainer ---------------------------- */

TaskContainer::~TaskContainer()
{
    MutexGuard guard(tasksLock);
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) task->finalize();
}
bool TaskContainer::addTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    const char* task_id = task->getId().c_str();
    if (!task_id || task_id[0] == '\0' || tasks.Exists(task_id)) return false;
    tasks.Insert(task_id, task);
    prioritySum += task->getPriority();
    return true;
}
bool TaskContainer::removeTask(std::string taskId)
{
    MutexGuard guard(tasksLock);
    
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return false;
    Task* task = tasks.Get(task_id);
    if (!task) return false;
    tasks.Delete(task_id);
    prioritySum -= task->getPriority();
    task->finalize();
    return true;
}
bool TaskContainer::hasTask(std::string taskId)
{
    MutexGuard guard(tasksLock);

    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return false;
    return true;
}
TaskGuard TaskContainer::getTask(std::string taskId)
{
    MutexGuard guard(tasksLock);
    
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return TaskGuard(0);
    Task* task = tasks.Get(task_id);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}
TaskGuard TaskContainer::getNextTask()
{
    MutexGuard guard(tasksLock);
    
    if (prioritySum <= 0) return TaskGuard(0);
    
    int count = 0;
    int random = ((rand()&0x7fffffff)%prioritySum)+1;
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) 
        { 
            count += task->getPriority();
            if (random <= count && !task->isFinalizing()) return TaskGuard(task);    
        }
    
    return TaskGuard(0);
}
void TaskContainer::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) task->resetWaiting();
}

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : TaskProcessorAdapter(), Thread(),
        logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), bNeedExit(false), taskTablesPrefix(0), 
                dsInternalName(0), dsInternal(0), dsIntConnection(0), messageSender(0)
{
    logger.info("Loading ...");

    std::auto_ptr<ConfigView> threadPoolCfgGuard(config->getSubConfig("TasksThreadPool"));
    ConfigView* threadPoolCfg = threadPoolCfgGuard.get();
    manager.init(threadPoolCfg); // loads up thread pool for tasks
    
    std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
    ConfigView* providerCfg = providerCfgGuard.get();
    provider.init(providerCfg);
    
    dsInternalName = providerCfg->getString("dsInternal", 
                                            "Internal DataSource driver name missed.", true);
    dsInternal = provider.getDataSource(dsInternalName);
    if (!dsInternal)
        throw ConfigException("Failed to obtail internal DataSource driver '%s'", dsInternalName);
    dsIntConnection = dsInternal->getConnection();
    if (!dsIntConnection)
        throw ConfigException("Failed to obtain connection to internal data source.");

    logger.info("Internal DataSource driver '%s' obtained.", dsInternalName);
    
    logger.info("Loading tasks ...");
    std::auto_ptr<ConfigView> tasksCfgGuard(config->getSubConfig("Tasks"));
    ConfigView* tasksCfg = tasksCfgGuard.get();
    switchTimeout = tasksCfg->getInt("switchTimeout");
    taskTablesPrefix = tasksCfg->getString("taskTablesPrefix");
    if (switchTimeout <= 0) 
        throw ConfigException("Task switch timeout should be positive");
    
    std::auto_ptr< std::set<std::string> > setGuard(tasksCfg->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* taskId = (const char *)i->c_str();
            if (!taskId || taskId[0] == '\0')
                throw ConfigException("Task id empty or wasn't specified");
            logger.info("Loading task '%s' ...", taskId);
            
            std::auto_ptr<ConfigView> taskConfigGuard(tasksCfg->getSubConfig(taskId));
            ConfigView* taskConfig = taskConfigGuard.get();
            const char* dsId = taskConfig->getString("dsId");
            if (!dsId || dsId[0] == '\0')
                throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                      taskId);
            DataSource* taskDs = provider.getDataSource(dsId);
            if (!taskDs)
                throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                      dsId, taskId);
            
            if (!container.addTask(new Task(taskConfig, taskId, taskTablesPrefix, taskDs, dsInternal)))
                throw ConfigException("Failed to add task. Task with id '%s' already registered.",
                                      taskId);
        }
        catch (ConfigException& exc)
        {
            logger.error("Load of tasks failed ! Config exception: %s", exc.what());
            throw;
        }
    }
    logger.info("Tasks loaded.");

    logger.info("Loading task schedules ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Schedules"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(this, schedulerCfg);
    logger.info("Task schedules loaded.");
    
    logger.info("Load success.");
    
    scheduler.Start();
}
TaskProcessor::~TaskProcessor()
{
    scheduler.Stop();
    this->Stop();

    if (dsIntConnection) dsInternal->freeConnection(dsIntConnection);
    if (dsInternalName) delete dsInternalName;
    if (taskTablesPrefix) delete taskTablesPrefix;
}
void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        container.resetWaitingTasks();
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
        logger.info("Started.");
    }
}
void TaskProcessor::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        logger.info("Stopping ...");
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        taskIdsBySeqNum.Empty();
        logger.info("Stoped.");
    }
}
int TaskProcessor::Execute()
{
    bool first = true;
    while (!bNeedExit)
    {
        awake.Wait((first) ? 100:switchTimeout);
        if (bNeedExit) break;
        try 
        {
            MainLoop(); first = false;
        } 
        catch (std::exception& exc) 
        {
            awake.Wait(0); first = true;
            logger.error("Exception occurred during tasks execution : %s", exc.what());
        }
    }
    exited.Signal();
    return 0;
}

void TaskProcessor::MainLoop()
{
    __require__(dsIntConnection);

    TaskGuard taskGuard = container.getNextTask(); 
    Task* task = taskGuard.get();
    if (!task || !task->isEnabled()) return;
    
    TaskInfo info = task->getInfo();
    time_t ct = time(NULL);
    if (ct >=  info.endDate) return;

    if (info.activePeriodStart > 0 && info.activePeriodEnd > 0)
    {
        if (info.activePeriodStart < info.activePeriodEnd) return;

        tm dt; localtime_r(&ct, &dt);
        
        dt.tm_isdst = -1;
        dt.tm_hour = info.activePeriodStart/3600;
        dt.tm_min  = (info.activePeriodStart%3600)/60;
        dt.tm_sec  = (info.activePeriodStart%3600)%60;
        time_t apst = mktime(&dt);

        dt.tm_isdst = -1;
        dt.tm_hour = info.activePeriodEnd/3600;
        dt.tm_min  = (info.activePeriodEnd%3600)/60;
        dt.tm_sec  = (info.activePeriodEnd%3600)%60;
        time_t apet = mktime(&dt);

        if (ct < apst || ct > apet) return;
    }
    
    Message message;
    {
        MutexGuard icGuard(dsIntConnectionLock);
        if (!task->getNextMessage(dsIntConnection, message)) return;
    }

    logger.debug("Message #%lld for '%s': %s", 
                 message.id, message.abonent.c_str(), message.message.c_str());

    MutexGuard msGuard(messageSenderLock);
    if (messageSender)
    {
        int seqNum = messageSender->sendMessage(message.abonent, message.message, info);
        MutexGuard snGuard(taskIdsBySeqNumLock);
        if (taskIdsBySeqNum.Exist(seqNum))
        {
            logger.warn("Sequence id=%d was already used !", seqNum);
            taskIdsBySeqNum.Delete(seqNum);
        }
        taskIdsBySeqNum.Insert(seqNum, TaskMsgId(info.id, message.id));
    }
}

const char* CREATE_ID_MAPPING_STATEMENT_ID = "CREATE_ID_MAPPING_STATEMENT_ID";
const char* CREATE_ID_MAPPING_STATEMENT_SQL = (const char*)
"INSERT INTO INFOSME_ID_MAPPING (ID, SMSC_ID, TASK_ID) VALUES (:ID, :SMSC_ID, :TASK_ID)";

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, std::string smscId)
{
    TaskMsgId* tmIds = 0;
    
    {   // Get taskId & msgId by seqNum
        MutexGuard snGuard(taskIdsBySeqNumLock);
        if (!(tmIds = taskIdsBySeqNum.GetPtr(seqNum))) {
            logger.warn("Sequence number=%d is unknown !", seqNum);
            return;
        }
        taskIdsBySeqNum.Delete(seqNum);
    }
    
    TaskGuard taskGuard = container.getTask(tmIds->taskId); 
    Task* task = taskGuard.get();
    if (!task) {
        logger.warn("Unable to locate task '%s' for sequence number=%d", 
                    tmIds->taskId.c_str(), seqNum);
        return;
    }
    TaskInfo info = task->getInfo();

    MutexGuard icGuard(dsIntConnectionLock);
    if (!accepted)
    {
        if (retry && info.retryOnFail && info.retryTime > 0)
             if (!task->doFailed(dsIntConnection, tmIds->msgId))
                logger.warn("Message #%lld not found (doRetry).", tmIds->msgId);
        else if (!task->doFailed(dsIntConnection, tmIds->msgId))
                logger.warn("Message #%lld not found (doFailed).", tmIds->msgId);
    } 
    else
    {
        if (!task->doEnroute(dsIntConnection, tmIds->msgId))
            logger.warn("Message #%lld not found (doEnroute).", tmIds->msgId);
        else
        {
            // TODO: try catch block here !!!
            Statement* createMapping = Task::getStatement(dsIntConnection, 
                                                          CREATE_ID_MAPPING_STATEMENT_ID,
                                                          CREATE_ID_MAPPING_STATEMENT_SQL);
            if (!createMapping) {
                logger.error("Failed to create statement for ids mapping.");
                return;
            }
            createMapping->setUint64(1, tmIds->msgId);
            createMapping->setString(2, smscId.c_str());
            createMapping->setString(3, info.id.c_str());
            createMapping->executeUpdate();
        }
    }
    // TODO: Implement time dependent commit on dsIntConnection.
}

const char* GET_ID_MAPPING_STATEMENT_ID = "GET_ID_MAPPING_STATEMENT_ID";
const char* GET_ID_MAPPING_STATEMENT_SQL = (const char*)
"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID";

const char* DELETE_ID_MAPPING_STATEMENT_ID = "DELETE_ID_MAPPING_STATEMENT_ID";
const char* DELETE_ID_MAPPING_STATEMENT_SQL = (const char*)
"DELETE FROM INFOSME_ID_MAPPING WHERE ID=:ID";

void TaskProcessor::precessReceipt (std::string smscId, bool delivered)
{
    /* TODO:
            1) Lock dsIntConnection
            2) Get taskId, msgId by smscId via SELECT FROM INFOSME_ID_MAPPING ...
            3) DELETE FROM INFOSME_ID_MAPPING ...
            4) call processReceipt(dsIntConnection, msgId, )
    */
}

}}

