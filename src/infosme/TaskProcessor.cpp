
#include "TaskProcessor.h"
#include <exception>

extern bool isMSISDNAddress(const char* string);

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
bool TaskContainer::putTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    const char* task_id = task->getId().c_str();
    if (!task_id || task_id[0] == '\0' || tasks.Exists(task_id)) return false;
    tasks.Insert(task_id, task);
    prioritySum += task->getPriority();
    return true;
}
bool TaskContainer::addTask(Task* task)
{
    __require__(task);
    bool result = putTask(task);
    if (result) task->createTable();
    return result;
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
void TaskContainer::resetWaitingTasks(Connection* connection)
{
    MutexGuard guard(tasksLock);
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) task->resetWaiting(connection);
}

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : TaskProcessorAdapter(), InfoSmeAdmin(), Thread(),
        logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), bNeedExit(false), taskTablesPrefix(0), 
                dsInternalName(0), dsInternal(0), dsIntConnection(0), dsCommitInterval(1),
                    messageSender(0), protocolId(0), svcType(0), address(0)
{
    logger.info("Loading ...");

    address = config->getString("Address");
    if (!address || !isMSISDNAddress(address))
        throw ConfigException("Address string '%s' is invalid", address ? address:"-");
    
    dsCommitInterval = config->getInt("dsIntCommitInterval");
    if (dsCommitInterval <= 0)
        throw ConfigException("dsIntCommitInterval is invalid, should be positive (in seconds)");

    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
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
            
            if (!container.putTask(new Task(taskConfig, taskId, taskTablesPrefix, taskDs, dsInternal)))
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
void TaskProcessor::dsInternalCommit(bool force)
{
    time_t currentTime = time(NULL);
    static time_t nextTime = currentTime + dsCommitInterval;
    
    try 
    {
        if (force || nextTime <= currentTime) {
            if (dsIntConnection) dsIntConnection->commit();
            logger.debug("dsInternalCommit !!!\n");
            nextTime = currentTime + dsCommitInterval;
        }
    }
    catch (Exception& exc) {
        logger.error("Failed to commit updates on internal data source. "
                     "Details: %s", exc.what());
    }
    catch (...) {
        logger.error("Failed to commit updates on internal data source.");
    }
}

void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        {
            MutexGuard icGuard(dsIntConnectionLock);
            container.resetWaitingTasks(dsIntConnection);
            dsInternalCommit(true);
        }
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
        {
            MutexGuard icGuard(dsIntConnectionLock);
            dsInternalCommit(true);
        }
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
    if (info.endDate > 0 && ct >=  info.endDate) return;

    if (info.activePeriodStart > 0 && info.activePeriodEnd > 0)
    {
        if (info.activePeriodStart > info.activePeriodEnd) return;
        
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
        if (!task->getNextMessage(dsIntConnection, message)) {
            //logger.debug("No messages found for task '%s'", info.id.c_str());
            return;
        }
        dsInternalCommit();
    }

    logger.debug("Sending message #%lld for '%s': %s", 
                 message.id, message.abonent.c_str(), message.message.c_str());

    MutexGuard msGuard(messageSenderLock);
    if (messageSender)
    {
        int seqNum = 0;
        if (!messageSender->send(message.abonent, message.message, info, seqNum)) {
            logger.error("Failed to send message #%lld for '%s'", 
                         message.id, message.abonent.c_str());
            return;
        }
        
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
    __require__(dsIntConnection);

    logger.debug("Processing resp: seqNum=%d, accepted=%d, retry=%d", seqNum, accepted, retry);

    TaskMsgId tmIds;
    
    {   // Get taskId & msgId by seqNum
        TaskMsgId* tmIdsPtr = 0;
        MutexGuard snGuard(taskIdsBySeqNumLock);
        if (!(tmIdsPtr = taskIdsBySeqNum.GetPtr(seqNum))) {
            logger.warn("processResponce(): Sequence number=%d is unknown !", seqNum);
            return;
        }
        tmIds = *tmIdsPtr;
        taskIdsBySeqNum.Delete(seqNum);
        //logger.debug("Task id=%s, msgid=%lld for seqNum=%d", 
        //             tmIds.taskId.c_str(), tmIds.msgId, seqNum);
    }
    
    TaskGuard taskGuard = container.getTask(tmIds.taskId); 
    Task* task = taskGuard.get();
    if (!task) {
        logger.warn("Unable to locate task '%s' for sequence number=%d", 
                    tmIds.taskId.c_str(), seqNum);
        return;
    }
    TaskInfo info = task->getInfo();

    MutexGuard icGuard(dsIntConnectionLock);
    if (!accepted)
    {
        if (retry && info.retryOnFail && info.retryTime > 0) {
            if (!task->doRetry(dsIntConnection, tmIds.msgId))
               logger.warn("Message #%lld not found (doRetry).", tmIds.msgId);
        }
        else if (!task->doFailed(dsIntConnection, tmIds.msgId))
                logger.warn("Message #%lld not found (doFailed).", tmIds.msgId);
    } 
    else
    {
        if (task->doEnroute(dsIntConnection, tmIds.msgId))
        {
            try
            {
                Statement* createMapping = Task::getStatement(dsIntConnection, 
                                                              CREATE_ID_MAPPING_STATEMENT_ID,
                                                              CREATE_ID_MAPPING_STATEMENT_SQL);
                if (!createMapping)
                    throw Exception("processResponce(): Failed to create statement for ids mapping.");
                
                createMapping->setUint64(1, tmIds.msgId);
                createMapping->setString(2, smscId.c_str());
                createMapping->setString(3, info.id.c_str());
                
                createMapping->executeUpdate();
            }
            catch (Exception& exc) {
                logger.error("Task '%s'. Failed to process responce. Ids mapping failure. "
                             "Details: %s", info.id.c_str(), exc.what());
            }
            catch (...) {
                logger.error("Task '%s'. Failed to process responce. Ids mapping failure.", 
                             info.id.c_str());
            }
        }
        else
            logger.warn("Message #%lld not found (doEnroute).", tmIds.msgId);
    }
    
    dsInternalCommit();
}

const char* GET_ID_MAPPING_STATEMENT_ID = "GET_ID_MAPPING_STATEMENT_ID";
const char* GET_ID_MAPPING_STATEMENT_SQL = (const char*)
"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID FOR UPDATE";

const char* DEL_ID_MAPPING_STATEMENT_ID = "DEL_ID_MAPPING_STATEMENT_ID";
const char* DEL_ID_MAPPING_STATEMENT_SQL = (const char*)
"DELETE FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID";

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry)
{
    __require__(dsIntConnection);

    logger.debug("Processing recpt: smscId=%s, delivered=%d, retry=%d",
                 smscId.c_str(), delivered, retry);

    MutexGuard icGuard(dsIntConnectionLock);

    try
    {
        Statement* getMapping = Task::getStatement(dsIntConnection, 
                                                   GET_ID_MAPPING_STATEMENT_ID,
                                                   GET_ID_MAPPING_STATEMENT_SQL);
        Statement* delMapping = Task::getStatement(dsIntConnection, 
                                                   DEL_ID_MAPPING_STATEMENT_ID,
                                                   DEL_ID_MAPPING_STATEMENT_SQL);
        if (!getMapping || !delMapping)
            throw Exception("processReceipt(): Failed to create statement for ids mapping.");
       
        getMapping->setString(1, smscId.c_str());
        std::auto_ptr<ResultSet> rsGuard(getMapping->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs || !rs->fetchNext())
            throw Exception("processReceipt(): Failed to obtain result set for ids mapping.");

        uint64_t msgId = rs->getUint64(1);
        std::string taskId = rs->getString(2);
        
        delMapping->setString(1, smscId.c_str());
        delMapping->executeUpdate();
        
        TaskGuard taskGuard = container.getTask(taskId); 
        Task* task = taskGuard.get();
        if (!task)
            throw Exception("processReceipt(): Unable to locate task '%s' for smscId=%s",
                            taskId.c_str(), smscId.c_str());
        TaskInfo info = task->getInfo();
        
        if (!delivered)
        {
            if (retry && info.retryOnFail && info.retryTime > 0) {
                if (!task->doRetry(dsIntConnection, msgId))
                   logger.warn("Message #%lld not found (doRetry).", msgId);
            }
            else if (!task->doFailed(dsIntConnection, msgId))
                    logger.warn("Message #%lld not found (doFailed).", msgId);
        }
        else if (!task->doDelivered(dsIntConnection, msgId))
                    logger.warn("Message #%lld not found (doDelivered).", msgId);
    }
    catch (Exception& exc) {
        logger.error("Failed to process receipt. Ids mapping failure. "
                     "Details: %s", exc.what());
    }
    catch (...) {
        logger.error("Failed to process receipt. Ids mapping failure.");
    }
    
    dsInternalCommit();
}

}}

