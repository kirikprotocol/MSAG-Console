
#include "TaskProcessor.h"
#include <exception>

extern bool isMSISDNAddress(const char* string);

namespace smsc { namespace infosme 
{

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : TaskProcessorAdapter(), InfoSmeAdmin(), Thread(),
        logger(Logger::getCategory("smsc.infosme.TaskProcessor")), 
            bStarted(false), bNeedExit(false), taskTablesPrefix(0), 
                dsInternal(0), dsIntConnection(0), messageSender(0), 
                    responceWaitTime(0), receiptWaitTime(0), dsStatConnection(0),
                        statistics(0), protocolId(0), svcType(0), address(0)
{
    logger.info("Loading ...");

    address = config->getString("Address");
    if (!address || !isMSISDNAddress(address))
        throw ConfigException("Address string '%s' is invalid", address ? address:"-");
    
    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
    responceWaitTime = parseTime(config->getString("responceWaitTime"));
    if (responceWaitTime <= 0) 
        throw ConfigException("Invalid value for 'responceWaitTime' parameter.");
    receiptWaitTime = parseTime(config->getString("receiptWaitTime"));
    if (receiptWaitTime <= 0) 
        throw ConfigException("Invalid value for 'receiptWaitTime' parameter.");
    switchTimeout = config->getInt("tasksSwitchTimeout");
    taskTablesPrefix = config->getString("tasksTablesPrefix");
    if (switchTimeout <= 0) 
        throw ConfigException("Task switch timeout should be positive");
    
    std::auto_ptr<ConfigView> tasksThreadPoolCfgGuard(config->getSubConfig("TasksThreadPool"));
    taskManager.init(tasksThreadPoolCfgGuard.get()); // loads up thread pool for tasks
    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("EventsThreadPool"));
    taskManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
    provider.init(providerCfgGuard.get());
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("systemDataSource"));
    dsInternal = provider.createDataSource(dsIntCfgGuard.get());
    if (!dsInternal)
        throw ConfigException("Failed to loadup internal DataSource");
    
    dsIntConnection = dsInternal->getConnection();
    dsStatConnection = dsInternal->getConnection();
    if (!dsIntConnection || !dsStatConnection)
        throw ConfigException("Failed to obtain connection(s) to internal data source.");
    
    logger.info("Loading tasks ...");
    std::auto_ptr<ConfigView> tasksCfgGuard(config->getSubConfig("Tasks"));
    ConfigView* tasksCfg = tasksCfgGuard.get();
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
            
            if (!putTask(new Task(taskConfig, taskId, taskTablesPrefix, taskDs, dsInternal)))
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
    
    statistics = new StatisticsManager(dsStatConnection);
    if (statistics) statistics->Start();
    scheduler.Start();
}
TaskProcessor::~TaskProcessor()
{
    scheduler.Stop();
    this->Stop();
    taskManager.Stop();
    eventManager.Stop();
    
    {
        MutexGuard guard(tasksLock);
        char* key = 0; Task* task = 0; tasks.First();
        while (tasks.Next(key, task))
            if (task) task->finalize();
        tasks.Empty();
    }

    if (statistics) delete statistics;
    if (dsStatConnection) dsInternal->freeConnection(dsStatConnection);

    if (taskTablesPrefix) delete taskTablesPrefix;
    if (dsIntConnection) dsInternal->freeConnection(dsIntConnection);
    if (dsInternal) delete dsInternal;
}

bool TaskProcessor::putTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    const char* task_id = task->getId().c_str();
    if (!task_id || task_id[0] == '\0' || tasks.Exists(task_id)) return false;
    tasks.Insert(task_id, task);
    awake.Signal();
    return true;
}
bool TaskProcessor::addTask(Task* task)
{
    __require__(task);
    bool result = putTask(task);
    if (result) result = task->createTable();
    return result;
}
bool TaskProcessor::remTask(std::string taskId)
{
    Task* task = 0;
    {
        MutexGuard guard(tasksLock);
        const char* task_id = taskId.c_str();
        if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return false;
        task = tasks.Get(task_id);
        tasks.Delete(task_id);
        if (!task) return false;
        awake.Signal();
    }
    if (task) task->finalize();
    return true;
}
bool TaskProcessor::delTask(std::string taskId)
{
    Task* task = 0;
    {
        MutexGuard guard(tasksLock);
        const char* task_id = taskId.c_str();
        if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return false;
        task = tasks.Get(task_id);
        tasks.Delete(task_id);
        if (!task) return false;
        awake.Signal();
    }
    return (task) ? task->destroy():false;
}
bool TaskProcessor::hasTask(std::string taskId)
{
    MutexGuard guard(tasksLock);

    const char* task_id = taskId.c_str();
    return (task_id && task_id[0] != '\0' && tasks.Exists(task_id));
}
TaskGuard TaskProcessor::getTask(std::string taskId)
{
    MutexGuard guard(tasksLock);
    
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0' || !tasks.Exists(task_id)) return TaskGuard(0);
    Task* task = tasks.Get(task_id);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}

void TaskProcessor::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    MutexGuard icGuard(dsIntConnectionLock);
    
    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task) task->resetWaiting(dsIntConnection);
}
void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        {
            MutexGuard snGuard(taskIdsBySeqNumLock);
            taskIdsBySeqNum.Empty();
        }
        resetWaitingTasks();
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
        logger.info("Stoped.");
    }
}
int TaskProcessor::Execute()
{
    Array<TaskGuard *> taskGuards;

    while (!bNeedExit)
    {
        time_t currentTime = time(NULL);
        
        {
            MutexGuard guard(tasksLock);
            char* key = 0; Task* task = 0; tasks.First();
            while (!bNeedExit && tasks.Next(key, task))
                if (task && task->isReady(currentTime)) {
                    taskGuards.Push(new TaskGuard(task));
                    task->currentPriorityFrameCounter = 0;
                }
        }

        int processed = 0;
        while (taskGuards.Count()>0)
        {
            TaskGuard* taskGuard = 0;
            taskGuards.Shift(taskGuard);
            if (!taskGuard) continue;

            if (!bNeedExit)
            {
                Task* task = taskGuard->get();
                if (task && !task->isFinalizing() &&
                    task->currentPriorityFrameCounter < task->getPriority())
                {
                    task->currentPriorityFrameCounter++;
                    if (!processTask(task))
                        task->currentPriorityFrameCounter = task->getPriority();
                    else processed++;
                }
            }
            delete taskGuard;
        }

        processWaitingEvents(currentTime); // ?? or time(NULL)
        if (!bNeedExit && processed <= 0) awake.Wait(switchTimeout);
    }
    exited.Signal();
    return 0;
}

bool TaskProcessor::processTask(Task* task)
{
    __require__(task && dsIntConnection);
     
    TaskInfo info = task->getInfo();
    Message message;
    {
        MutexGuard icGuard(dsIntConnectionLock);
        if (!task->getNextMessage(dsIntConnection, message)) {
            //logger.debug("No messages found for task '%s'", info.id.c_str());
            return false;
        }
    }

    //logger.debug("Sending message #%lld for '%s': %s", 
    //             message.id, message.abonent.c_str(), message.message.c_str());

    MutexGuard msGuard(messageSenderLock);
    if (messageSender)
    {
        int seqNum = messageSender->getSequenceNumber();
        {
            {
                MutexGuard snGuard(taskIdsBySeqNumLock);
                if (taskIdsBySeqNum.Exist(seqNum))
                {
                    logger.warn("Sequence id=%d was already used !", seqNum);
                    taskIdsBySeqNum.Delete(seqNum);
                }
                taskIdsBySeqNum.Insert(seqNum, TaskMsgId(info.id, message.id));
            }
            MutexGuard respGuard(responceWaitQueueLock);
            responceWaitQueue.Push(ResponceTimer(time(NULL)+responceWaitTime, seqNum));
        }
        
        if (!messageSender->send(message.abonent, message.message, info, seqNum))
        {
            logger.error("Failed to send message #%lld for '%s'", 
                         message.id, message.abonent.c_str());
            
            MutexGuard snGuard(taskIdsBySeqNumLock);
            if (taskIdsBySeqNum.Exist(seqNum)) taskIdsBySeqNum.Delete(seqNum);
            return false;
        }
        //logger.debug("Sent message #%lld for '%s'", message.id, message.abonent.c_str());
    }
    else
    {
        logger.error("No messageSender defined !!!");
        return false;
    }

    return true;
}

void TaskProcessor::processWaitingEvents(time_t time)
{
    int count = 0;
    
    do
    {
        ResponceTimer timer;
        {
            MutexGuard respGuard(responceWaitQueueLock);
            if (responceWaitQueue.Count() > 0) {
                timer = responceWaitQueue[0];
                if (timer.timer > time) break;
                responceWaitQueue.Shift(timer);
            }
            else break;
        }

        bool needProcess = false;
        {
            MutexGuard guard(taskIdsBySeqNumLock);
            needProcess = taskIdsBySeqNum.Exist(timer.seqNum);
        }
        if (needProcess)
            processResponce(timer.seqNum, false, true, true, "", true);
        
        {
            MutexGuard respGuard(responceWaitQueueLock);
            count = responceWaitQueue.Count();
        }
    }
    while (!bNeedExit && count > 0);
    
    do
    {
        ReceiptTimer timer;
        {
            MutexGuard recptGuard(receiptWaitQueueLock);
            if (receiptWaitQueue.Count() > 0) {
                timer = receiptWaitQueue[0];
                if (timer.timer > time) break;
                receiptWaitQueue.Shift(timer);
            } 
            else break;
        }

        bool needProcess = false;
        {
            MutexGuard guard(receiptsLock);
            ReceiptData* receiptPtr = receipts.GetPtr(timer.smscId.c_str());
            if (receiptPtr) logger.warn("%s for smscId=%s wasn't received and timed out!", 
                                        ((receiptPtr->receipted) ? "Receipt":"Responce"),
                                        timer.smscId.c_str());
            
        }
        if (needProcess)
            processReceipt(timer.smscId, false, true, true);

        {
            MutexGuard recptGuard(receiptWaitQueueLock);
            count = receiptWaitQueue.Count();
        }
    }
    while (!bNeedExit && count > 0);
}

void TaskProcessor::processMessage(Task* task, Connection* connection, uint64_t msgId,
                                   bool delivered, bool retry, bool immediate)
{
    __require__(task && connection);

    if (delivered)
    {
        task->deleteMessage(msgId, connection);
        statistics->incDelivered(task->getId());
    }
    else
    {
        bool needDelete = true;
        TaskInfo info = task->getInfo();
        if (retry && (immediate || (info.retryOnFail && info.retryTime > 0)))
        {
            time_t nextTime = time(NULL)+((immediate) ? 0:info.retryTime);
            if (info.endDate <= 0 || (info.endDate > 0 && info.endDate >= nextTime))
            {
                if (!task->retryMessage(msgId, nextTime, connection)) {
                    logger.warn("Message #%lld not found for retry.", msgId);
                    statistics->incFailed(info.id);
                } else {
                    needDelete = false;
                    if (!immediate) statistics->incRetried(info.id);
                }
            }
            else statistics->incFailed(info.id);
        }
        else statistics->incFailed(info.id);
        if (needDelete) task->deleteMessage(msgId, connection);
    }
}

const char* CREATE_ID_MAPPING_STATEMENT_ID = "CREATE_ID_MAPPING_STATEMENT_ID";
const char* CREATE_ID_MAPPING_STATEMENT_SQL = (const char*)
"INSERT INTO INFOSME_ID_MAPPING (ID, SMSC_ID, TASK_ID) VALUES (:ID, :SMSC_ID, :TASK_ID)";

const char* GET_ID_MAPPING_STATEMENT_ID = "GET_ID_MAPPING_STATEMENT_ID";
const char* GET_ID_MAPPING_STATEMENT_SQL = (const char*)
"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID FOR UPDATE";

const char* DEL_ID_MAPPING_STATEMENT_ID = "DEL_ID_MAPPING_STATEMENT_ID";
const char* DEL_ID_MAPPING_STATEMENT_SQL = (const char*)
"DELETE FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID";

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    std::string smscId, bool internal)
{
    if (!internal) logger.debug("Responce: seqNum=%d, accepted=%d, retry=%d, immediate=%d",
                                seqNum, accepted, retry, immediate);
    else logger.debug("Responce for seqNum=%d is timed out.", seqNum);

    TaskMsgId tmIds;
    {   
        TaskMsgId* tmIdsPtr = 0;
        MutexGuard snGuard(taskIdsBySeqNumLock);
        if (!(tmIdsPtr = taskIdsBySeqNum.GetPtr(seqNum))) {
            if (!internal) logger.warn("processResponce(): Sequence number=%d is unknown !", seqNum);
            return;
        }
        tmIds = *tmIdsPtr;
        taskIdsBySeqNum.Delete(seqNum);
    }
    
    TaskGuard taskGuard = getTask(tmIds.taskId); 
    Task* task = taskGuard.get();
    if (!task) {
        if (!internal) logger.warn("Unable to locate task '%s' for sequence number=%d", 
                                   tmIds.taskId.c_str(), seqNum);
        return;
    }
    TaskInfo info = task->getInfo();

    if (!accepted || internal)
    {
        bool needDelete = true;
        if (retry && (immediate || (info.retryOnFail && info.retryTime > 0)))
        {
            time_t nextTime = time(NULL)+((immediate) ? 0:info.retryTime);
            if (info.endDate <= 0 || (info.endDate > 0 && info.endDate >= nextTime))
            {
                if (!task->retryMessage(tmIds.msgId, nextTime)) {
                    logger.warn("Message #%lld not found for retry.", tmIds.msgId);
                    statistics->incFailed(tmIds.taskId);
                } else {
                    needDelete = false;
                    if (!immediate) statistics->incRetried(tmIds.taskId);
                }
            }
            else statistics->incFailed(tmIds.taskId);
        }
        else statistics->incFailed(tmIds.taskId);
        if (needDelete) task->deleteMessage(tmIds.msgId);
    }
    else
    {
        if (info.transactionMode) {
            statistics->incDelivered(tmIds.taskId);
            task->deleteMessage(tmIds.msgId);
            return;
        }
        
        Connection* connection = 0;
        const char* smsc_id = smscId.c_str();
        
        try
        {
            connection = dsInternal->getConnection();
            if (!connection)
                throw Exception("processResponce(): Failed to obtain connection to internal data source.");

            ReceiptData receipt; // receipt.receipted = false
            {
                MutexGuard guard(receiptsLock);
                ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
                if (receiptPtr) receipt = *receiptPtr;
                else {
                    receipts.Insert(smsc_id, receipt);
                    MutexGuard recptGuard(receiptWaitQueueLock);
                    receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smscId));
                }
            }

            bool idMappingCreated = false;
            if (!receipt.receipted)
            {
                if (!task->enrouteMessage(tmIds.msgId, connection))
                    throw Exception("Message #%lld not found (doEnroute).", tmIds.msgId);

                Statement* createMapping = connection->getStatement(CREATE_ID_MAPPING_STATEMENT_ID,
                                                                    CREATE_ID_MAPPING_STATEMENT_SQL);
                if (!createMapping)
                    throw Exception("processResponce(): Failed to create statement for ids mapping.");

                createMapping->setUint64(1, tmIds.msgId);
                createMapping->setString(2, smsc_id);
                createMapping->setString(3, info.id.c_str());
                createMapping->executeUpdate();
                connection->commit();
            }

            {
                MutexGuard guard(receiptsLock);
                ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
                if (receiptPtr) {
                    receipt = *receiptPtr;
                    receipts.Delete(smsc_id);
                }
                else receipt.receipted = false;
            }

            if (receipt.receipted) // receipt already come
            {
                logger.debug("Receipt come when responce is in process");
                if (idMappingCreated)
                {
                    Statement* delMapping = connection->getStatement(DEL_ID_MAPPING_STATEMENT_ID,
                                                                     DEL_ID_MAPPING_STATEMENT_SQL);
                    if (!delMapping)
                        throw Exception("processResponce(): Failed to create statement for ids mapping.");

                    delMapping->setString(1, smsc_id);
                    delMapping->executeUpdate();
                }
                
                processMessage(task, connection, tmIds.msgId, receipt.delivered, receipt.retry);
                connection->commit();
            }
        }
        catch (Exception& exc) {
            try { if (connection) connection->rollback(); }
            catch (Exception& exc) {
                logger.error("Failed to roolback transaction on internal data source. "
                             "Details: %s", exc.what());
            } catch (...) {
                logger.error("Failed to roolback transaction on internal data source.");
            }
            logger.error("Failed to process responce. Details: %s", exc.what());
        }
        catch (...) {
            try { if (connection) connection->rollback(); }
            catch (Exception& exc) {
                logger.error("Failed to roolback transaction on internal data source. "
                             "Details: %s", exc.what());
            } catch (...) {
                logger.error("Failed to roolback transaction on internal data source.");
            }
            logger.error("Failed to process responce.");
        }

        if (connection) dsInternal->freeConnection(connection);
    }
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry, bool internal)
{
    const char* smsc_id = smscId.c_str();

    if (!internal) logger.debug("Receipt : smscId=%s, delivered=%d, retry=%d",
                                smsc_id, delivered, retry);
    else logger.debug("Responce/Receipt for smscId=%s is timed out. Cleanup.");
    
    if (!internal)
    {
        MutexGuard guard(receiptsLock);
        ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
        if (receiptPtr) // attach & return;
        {   
            receiptPtr->receipted = true;
            receiptPtr->delivered = delivered;
            receiptPtr->retry     = retry;
            return;
        }
        else
        {
            receipts.Insert(smsc_id, ReceiptData(true, delivered, retry));
            MutexGuard recptGuard(receiptWaitQueueLock);
            receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smscId));
        }
    }
    
    Connection* connection = 0;
    try
    {
        connection = dsInternal->getConnection();
        if (!connection)
            throw Exception("processReceipt(): Failed to obtain connection to internal data source.");

        Statement* getMapping = connection->getStatement(GET_ID_MAPPING_STATEMENT_ID,
                                                         GET_ID_MAPPING_STATEMENT_SQL);
        if (!getMapping)
            throw Exception("processReceipt(): Failed to create statement for ids mapping.");
    
        getMapping->setString(1, smsc_id);
        std::auto_ptr<ResultSet> rsGuard(getMapping->executeQuery());
        ResultSet* rs = rsGuard.get();
        
        if (rs && rs->fetchNext())
        {
            bool needProcess = false;
            {
                MutexGuard guard(receiptsLock);
                ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
                if (receiptPtr) { 
                    receipts.Delete(smsc_id);
                    needProcess = true;
                }
            }
            
            if (needProcess)
            {
                Statement* delMapping = connection->getStatement(DEL_ID_MAPPING_STATEMENT_ID,
                                                                 DEL_ID_MAPPING_STATEMENT_SQL);
                if (!delMapping)
                    throw Exception("processReceipt(): Failed to create statement for ids mapping.");

                uint64_t msgId = rs->getUint64(1);
                std::string taskId = rs->getString(2);
                delMapping->setString(1, smsc_id);
                delMapping->executeUpdate();

                TaskGuard taskGuard = getTask(taskId); 
                Task* task = taskGuard.get();
                if (!task)
                    throw Exception("processReceipt(): Unable to locate task '%s' for smscId=%s",
                                    taskId.c_str(), smsc_id);
                TaskInfo info = task->getInfo();

                processMessage(task, connection, msgId, delivered, retry, internal);
            }
            
            connection->commit();
        }
    }
    catch (Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Failed to process receipt. Details: %s", exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Failed to process receipt.");
    }
    
    if (connection) dsInternal->freeConnection(connection);
}

/* ------------------------ Admin interface implementation ------------------------ */ 

bool TaskProcessor::addTask(std::string taskId)
{
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0') return false;

    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%s", task_id);
        ConfigView taskConfig(config, taskSection);
        
        const char* ds_id = taskConfig.getString("dsId");
        if (!ds_id || ds_id[0] == '\0')
            throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                  task_id);
        DataSource* taskDs = provider.getDataSource(ds_id);
        if (!taskDs)
            throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                  ds_id, task_id);
        if (!addTask(new Task(&taskConfig, taskId, taskTablesPrefix, taskDs, dsInternal)))
            throw ConfigException("Failed to add task. Task with id '%s' already registered.",
                                  task_id);
        return true;
    }
    catch (Exception& exc) {
        logger.error("Failed to add task '%s'. Details: %s", task_id, exc.what());
    }
    catch (...) {}
    return false;
}
bool TaskProcessor::removeTask(std::string taskId)
{
    scheduler.removeTask(taskId);
    bool deleted = delTask(taskId);
    if (statistics) statistics->delStatistics(taskId);
    return deleted;
}
bool TaskProcessor::changeTask(std::string taskId)
{
    const char* task_id = taskId.c_str();
    if (!task_id || task_id[0] == '\0') return false;
    
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%s", task_id);
        ConfigView taskConfig(config, taskSection);
        
        const char* ds_id = taskConfig.getString("dsId");
        if (!ds_id || ds_id[0] == '\0')
            throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                  task_id);
        DataSource* taskDs = provider.getDataSource(ds_id);
        if (!taskDs)
            throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                  ds_id, task_id);
        if (!remTask(taskId))
            throw Exception("Failed to change task. Task with id '%s' wasn't registered.",
                            task_id);
        if (!putTask(new Task(&taskConfig, taskId, taskTablesPrefix, taskDs, dsInternal)))
            throw Exception("Failed to change task with id '%s'", task_id);
        return true;
    }
    catch (Exception& exc) {
        logger.error("Failed to change task '%s'. Details: %s", task_id, exc.what());
    }
    catch (...) {}
    return false;
}
bool TaskProcessor::startTask(std::string taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    return (task->isInProcess()) ? true:invokeBeginProcess(task);
}
bool TaskProcessor::stopTask(std::string taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    return (task->isInProcess()) ? true:invokeEndProcess(task);
}
Array<std::string> TaskProcessor::getStartedTasks()
{
    MutexGuard guard(tasksLock);
    
    Array<std::string> startedTasks;

    char* key = 0; Task* task = 0; tasks.First();
    while (tasks.Next(key, task))
        if (task && task->isInProcess()) 
            startedTasks.Push(task->getId());

    return startedTasks;
}
bool TaskProcessor::isTaskEnabled(std::string taskId)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    return (task && task->isEnabled());
}
bool TaskProcessor::setTaskEnabled(std::string taskId, bool enabled)
{
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) return false; 
    task->setEnabled(enabled);
    return true;
}
bool TaskProcessor::addSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') return false;

    Schedule* schedule = 0;
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char scheduleSection[1024];
        sprintf(scheduleSection, "InfoSme.Schedules.%s", schedule_id);
        ConfigView scheduleConfig(config, scheduleSection);
        
        schedule = Schedule::create(&scheduleConfig, scheduleId);
        bool result = scheduler.addSchedule(schedule);
        if (!result && schedule) delete schedule;
        return result;
    }
    catch (Exception& exc) {
        if (schedule) delete schedule;
        logger.error("Failed to add schedule '%s'. Details: %s", schedule_id, exc.what());
    }
    catch (...) {
        if (schedule) delete schedule;
    }
    return false;
}
bool TaskProcessor::removeSchedule(std::string scheduleId)
{
    return scheduler.removeSchedule(scheduleId);
}
bool TaskProcessor::changeSchedule(std::string scheduleId)
{
    const char* schedule_id = scheduleId.c_str();
    if (!schedule_id || schedule_id[0] == '\0') return false;

    Schedule* schedule = 0;
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char scheduleSection[1024];
        sprintf(scheduleSection, "InfoSme.Schedules.%s", schedule_id);
        ConfigView scheduleConfig(config, scheduleSection);
        
        schedule = Schedule::create(&scheduleConfig, scheduleId);
        bool result = scheduler.changeSchedule(scheduleId, schedule);
        if (!result && schedule) delete schedule;
        return result;
    }
    catch (Exception& exc) {
        if (schedule) delete schedule;
        logger.error("Failed to change schedule '%s'. Details: %s", schedule_id, exc.what());
    }
    catch (...) {
        if (schedule) delete schedule;
    }
    return false;
}


}}

