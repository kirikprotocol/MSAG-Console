
#include <exception>

#include "TaskProcessor.h"

extern bool isMSISDNAddress(const char* string);

namespace smsc { namespace mcisme 
{

static time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;

    //printf("%02d.%02d.%04d %02d:%02d:%02d = %ld\n", day, month, year, hour, minute, second, time);
    return mktime(&dt);
}
static time_t parseDate(const char* str)
{
    int year, month, day;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d", 
                    &day, &month, &year) != 3) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    
    //printf("%02d:%02d:%04d = %ld\n", day, month, year, time);
    return mktime(&dt);
}
static int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d:%02d:%02d", 
                    &hour, &minute, &second) != 3) return -1;
    
    //printf("%02d:%02d:%02d = %ld\n", hour, minute, second, time);
    return hour*3600+minute*60+second;
}

static void checkAddress(const char* address)
{
    static const char* ADDRESS_ERROR_NULL_MESSAGE    = "Destination address is undefined.";
    static const char* ADDRESS_ERROR_INVALID_MESSAGE = "Address '%s' is invalid";
    
    if (!address || address[0] == '\0')
        throw Exception(ADDRESS_ERROR_NULL_MESSAGE);
    if (!isMSISDNAddress(address))
        throw Exception(ADDRESS_ERROR_INVALID_MESSAGE, address);
}

int EventRunner::Execute()
{
    switch (method)
    {
    case processResponceMethod:
        processor.processResponce(seqNum, delivered, retry, immediate,
                                  replace, replace_failed, smscId);
        break;
    case processReceiptMethod:
        processor.processReceipt (smscId, delivered, retry);
        break;
    default:
        __trace2__("Invalid method '%d' invoked by event.", method);
        return -1;
    }

    // TODO: correct threads shutdown in EventManager !!!

    return 0;
}

/* ---------------------------- TaskProcessor ---------------------------- */

void TaskProcessor::initDataSource(ConfigView* config)
{
    try  {
        std::auto_ptr<char> dsIdentity(config->getString("type"));
        const char* dsIdentityStr = dsIdentity.get();
        ds = DataSourceFactory::getDataSource(dsIdentityStr);
        if (!ds) throw ConfigException("DataSource for '%s' identity "
                                       "wasn't registered !", dsIdentityStr);
        ds->init(config);
    } 
    catch (ConfigException& exc) {
        if (ds) delete ds; ds = 0;
        throw;
    }
}

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), MissedCallListener(), MCISmeAdmin(), 
        logger(Logger::getInstance("smsc.mcisme.TaskProcessor")), 
        protocolId(0), svcType(0), address(0), mciModule(0), messageSender(0), 
        ds(0), dsStatConnection(0), maxInQueueSize(10000), maxOutQueueSize(10000),
        bStarted(false), bInQueueOpen(false), bOutQueueOpen(false)
{
    smsc_log_info(logger, "Loading ...");

    address = config->getString("Address");
    if (!address || !isMSISDNAddress(address))
        throw ConfigException("Address string '%s' is invalid", address ? address:"-");
    
    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
    responcesTracker.init(this, config);

    maxInQueueSize  = config->getInt("inputQueueSize");
    maxOutQueueSize = config->getInt("outputQueueSize");

    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("SMPPThreadPool"));
    eventManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("DataSource"));
    initDataSource(dsIntCfgGuard.get());
    dsStatConnection = ds->getConnection();
    
    /* TODO: init StatisticsManager
    statistics = new StatisticsManager(dsStatConnection);
    if (statistics) statistics->Start();
    */
    Task::init(ds); // + statistics

    mciModule = new MCIModule(this);
    smsc_log_info(logger, "Load success.");
}
TaskProcessor::~TaskProcessor()
{
    eventManager.Stop();
    this->Stop();
    
    if (mciModule) delete mciModule;
    // if (statistics) delete statistics;
    if (dsStatConnection) ds->freeConnection(dsStatConnection);
    if (ds) delete ds;
}

void TaskProcessor::Start()
{
    MutexGuard guard(startLock);
    if (!bStarted)
    {
        smsc_log_info(logger, "Starting ...");

        if (!isMessageSenderAssigned()) {
            smsc_log_error(logger, "Failed to start processing. Message sender is undefined.");
            return;
        }
        
        openOutQueue();
        openInQueue();
        loadupTasks();
        
        Thread::Start();
        responcesTracker.Start();
        if (mciModule) mciModule->Start();
        
        bStarted = true;
        smsc_log_info(logger, "Started.");
    }
}
void TaskProcessor::Stop()
{
    MutexGuard  guard(startLock);
    if (bStarted)
    {
        smsc_log_info(logger, "Stopping ...");
        if (mciModule) mciModule->Stop();
        responcesTracker.Stop();
        
        closeInQueue();
        closeOutQueue();
        exitedEvent.Wait();
        unloadTasks();

        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
}
int TaskProcessor::Execute()
{
    static const char* ERROR_MESSAGE = "Failed to process missed call event. Details: %s";
    
    while (bInQueueOpen)
    {
        try
        {
            MissedCallEvent event;
            if (!getFromInQueue(event)) break;
            processEvent(event);

        } catch (std::exception& exc) {
            smsc_log_error(logger, ERROR_MESSAGE, exc.what());    
        } catch (...) {
            smsc_log_error(logger, ERROR_MESSAGE, "Cause is unknown");
        }
    }
    exitedEvent.Signal();
    return 0;
}
void TaskProcessor::Run()
{
    static const char* ERROR_MESSAGE = "Failed to process message. Details: %s";

    while (bOutQueueOpen)
    {
        try
        {
            Message message;
            if (!getFromOutQueue(message)) break;
            processMessage(message);

        } catch (std::exception& exc) {
            smsc_log_error(logger, ERROR_MESSAGE, exc.what());    
        } catch (...) {
            smsc_log_error(logger, ERROR_MESSAGE, "Cause is unknown");
        }
    }
}

void TaskProcessor::loadupTasks()
{
    MutexGuard guard(tasksMonitor);
    smsc_log_info(logger, "Loading processing tasks ...");
    lockedTasks.Empty();
    tasks = Task::loadupAll();
    smsc_log_info(logger, "Processing tasks loaded. Resending messages...");
    Message message;
    char* abonent=0; Task* task = 0; tasks.First();
    while (tasks.Next(abonent, task))
        if (task && task->formatMessage(message))
            putToOutQueue(message, true);
    smsc_log_info(logger, "All loaded tasks messages added to output queue.");
}
void TaskProcessor::unloadTasks()
{
    MutexGuard guard(tasksMonitor);
    smsc_log_info(logger, "Unloading processing tasks ...");
    char* abonent=0; Task* task = 0; tasks.First();
    while (tasks.Next(abonent, task)) 
        if (task) delete task;
    tasks.Empty(); lockedTasks.Empty();
    smsc_log_info(logger, "Processing tasks unloaded.");
}

Task* TaskProcessor::getTask(const char* abonent)
{
    MutexGuard guard(tasksMonitor);
    Task** taskPtr = 0;
    while (taskPtr = lockedTasks.GetPtr(abonent)) tasksMonitor.wait();
    taskPtr = tasks.GetPtr(abonent);
    if (taskPtr) lockedTasks.Insert(abonent, *taskPtr);
    return (taskPtr ? *taskPtr:0);
}
Task* TaskProcessor::getTask(const char* abonent, bool& newone)
{
    MutexGuard guard(tasksMonitor);
    Task** taskPtr = 0;
    while (taskPtr = lockedTasks.GetPtr(abonent)) tasksMonitor.wait();
    taskPtr = tasks.GetPtr(abonent);
    if (taskPtr) {
        lockedTasks.Insert(abonent, *taskPtr);
        newone = false;
        return *taskPtr;
    }
    
    Task* task = new Task(abonent);
    try { task->loadup(); }
    catch (...) { delete task; throw; }
    
    lockedTasks.Insert(abonent, task);
    tasks.Insert(abonent, task);
    newone = true;

    smsc_log_debug(logger, "Task for abonent %s created. Events=%d", abonent, task->eventsCount());

    return task;
}
bool TaskProcessor::freeTask(const char* abonent)
{
    MutexGuard guard(tasksMonitor);
    if (!lockedTasks.Exists(abonent)) return false;
    lockedTasks.Delete(abonent);
    tasksMonitor.notifyAll();
    return true;
}
bool TaskProcessor::delTask(const char* abonent)
{
    MutexGuard guard(tasksMonitor);
    Task** taskPtr = tasks.GetPtr(abonent);
    if (!taskPtr) return false;
    
    smsc_log_debug(logger, "Task for abonent %s killed. Events=%d", abonent, (*taskPtr)->eventsCount());

    delete *taskPtr;
    tasks.Delete(abonent);
    lockedTasks.Delete(abonent);
    tasksMonitor.notifyAll();
    return true;
}
class TaskAccessor
{
    TaskProcessor* processor;
    const char*    abonent;

public:

    TaskAccessor(TaskProcessor* processor) : processor(processor), abonent(0) {};
    ~TaskAccessor() { 
        if (processor && abonent) processor->freeTask(abonent);
    }
    
    Task* getTask(const char* _abonent, bool& newone) {
        Task* task = (processor) ? processor->getTask(_abonent, newone):0;
        this->abonent = (task) ? _abonent:0; // if no task => do not free it in destructor
        return task;
    }
    Task* getTask(const char* _abonent) {
        Task* task = (processor) ? processor->getTask(_abonent):0;
        this->abonent = (task) ? _abonent:0; // if no task => do not free it in destructor
        return task;
    }
    bool  delTask(const char* _abonent) {
        this->abonent = 0; // if task was deleted => do not free it in destructor
        return (processor) ? processor->delTask(_abonent):false;
    }
};

void TaskProcessor::lockSmscId(const char* smsc_id)
{
    MutexGuard guard(smscIdMonitor);
    while (lockedSmscIds.Exists(smsc_id)) smscIdMonitor.wait();
    lockedSmscIds.Insert(smsc_id, true);
}
void TaskProcessor::freeSmscId(const char* smsc_id)
{
    MutexGuard guard(smscIdMonitor);
    if (lockedSmscIds.Exists(smsc_id)) {
        lockedSmscIds.Delete(smsc_id);
        smscIdMonitor.notifyAll();
    }
}
class SmscIdAccessor
{
    TaskProcessor* processor;
    const char*    smsc_id;

public:

    SmscIdAccessor(TaskProcessor* _processor, const char* _smsc_id=0)
        : processor(_processor), smsc_id(_smsc_id) {
        if (processor && smsc_id) processor->lockSmscId(smsc_id);
    };
    ~SmscIdAccessor() { 
        if (processor && smsc_id) processor->freeSmscId(smsc_id);
    }
};

void TaskProcessor::processEvent(const MissedCallEvent& event)
{
    const char* abonent = event.to.c_str();
    checkAddress(abonent);

    Message message;
    {
        TaskAccessor taskAccessor(this);

        bool isNewTask = false;
        Task* task = taskAccessor.getTask(abonent, isNewTask); // possible loads task
        if (!task) throw Exception("Failed to obtain task for abonent: %s", abonent);
        task->addEvent(event); // adds event to task chain & inassigned into DB 
        smsc_log_debug(logger, "Event for %s added to %s task. Events=%d",
                       abonent, (isNewTask) ? "new":"existed", task->eventsCount());
        if (!isNewTask || !task->formatMessage(message)) return;
    }
    putToOutQueue(message);
}

void TaskProcessor::processMessage(const Message& message)
{
    /*smsc_log_debug(logger, "Sending message #%lld to '%s': %s", 
                   message.id, message.abonent.c_str(), message.message.c_str());*/

    MutexGuard msGuard(messageSenderLock);
    if (!messageSender) {
        smsc_log_error(logger, "No Message sender defined !!!");
        return;
    }

    int seqNum = messageSender->getSequenceNumber();
    if (!responcesTracker.putResponceData(seqNum, message)) {
        smsc_log_error(logger, "Sequence id=%d was already used !", seqNum);
        return;
    }

    if (!messageSender->send(seqNum, message)) {
        smsc_log_error(logger, "Failed to send message #%lld to '%s'.", message.id, message.abonent.c_str());
        //Message failedMessage; popResponceData(seqNum, failedMessage); ???
        return;
    }
    
    //smsc_log_debug(logger, "Sent message #%lld to '%s'.", message.id, message.abonent.c_str());
}

/* ------------------------ Main processing ------------------------ */ 

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    bool replace, bool replace_failed, std::string smscId)
{
    smsc_log_debug(logger, "Responce (%s): seqNum=%d, smscId=%s, accepted=%d, retry=%d, immediate=%d, rep_failed=%d",
                   replace ? "replace":"submit", seqNum, smscId.c_str(), accepted, retry, immediate, replace_failed);

    Message message; // get message by sequence number assigned in sender
    if (!responcesTracker.popResponceData(seqNum, message)) {   
        smsc_log_warn(logger, "Unable to locate message for sequence number: %d", seqNum);
        return;
    }
    
    if (replace) smscId=message.smsc_id;
    const char* smsc_id = (smscId.length() > 0) ? smscId.c_str():0;
    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id if exists
    if (smsc_id) smsc_log_debug(logger, "Responce lock smscId=%s", smsc_id);
    
    ReceiptData receipt; // check waiting receipt existance
    bool bWasReceipted = responcesTracker.popReceiptData(smsc_id, receipt);

    bool  needKillTask = false;
    bool  isMessageToSend = false;
    Message messageToSend;
    {
        // TODO: Events can be added while task in not locked by followed code !!!

        TaskAccessor taskAccessor(this);
        Task* task = taskAccessor.getTask(message.abonent.c_str());
        if (!task) { // Task MUST be in map for valid responce !!!
            smsc_log_error(logger, "Unable to locate task for abonent: %s", message.abonent.c_str());
            return;
        }

        if (!accepted)
        {
            if (retry)  // temporal error => keep task & retry with message (possible updated)
            { 
                if (task->formatMessage(messageToSend)) { 
                    isMessageToSend = true;
                    messageToSend.replace = message.replace; // TODO: ???
                    messageToSend.smsc_id = message.smsc_id; // TODO: ???
                }
                else smsc_log_error(logger, "Format message for retry failed for abonent: %s",
                                    message.abonent.c_str());
                if (!immediate) messageToSend.attempts = message.attempts+1;
                
                // do not roll current message here
            }
            else        // permanent error
            {
                if (replace && replace_failed)
                {
                    smsc_log_debug(logger, "Message #%lld (smsc_id=%s) replace error for abonent: %s",
                                   message.id, smsc_id ? smsc_id:"-", message.abonent.c_str());
                    if (smsc_id)
                    {
                        if (message.isFull() || bWasReceipted) {
                            // set new current message & skip task events for old message
                            task->waitReceipt(message.eventCount, smsc_id);
                        }
                        else {
                            // set new current message but keep old task events
                            task->waitReceipt(0, smsc_id);
                        }

                        // try send more events via submit next message (do not replace)
                        if (task->formatMessage(messageToSend))
                        { 
                            smsc_log_debug(logger, "Task has %d events more to send for abonent: %s (replace error)",
                                           messageToSend.eventCount, messageToSend.abonent.c_str());
                            isMessageToSend = true; messageToSend.replace = false;
                        } 
                        // if no more events to send => kill task
                        else needKillTask = true;
                    }
                    else needKillTask = true;
                }
                else
                {
                    smsc_log_debug(logger, "Message #%lld (smsc_id=%s) send error, deleting task for abonent: %s",
                                   message.id, smsc_id ? smsc_id:"-", message.abonent.c_str());
                    task->deleteMessages(); // TODO: ???
                    needKillTask = true;
                }
            }
        }
        else    // submit|replace was accepted by SMSC
        {
            if (message.eventCount < task->eventsCount()) // some events were added to task
            {
                if (message.isFull() || bWasReceipted) // sent message is full or was receipted
                {
                    // set new current message & skip task events for old message
                    task->waitReceipt(message.eventCount, smsc_id);
                    
                    // try send more events via submit next message 
                    if (task->formatMessage(messageToSend))
                    { 
                        smsc_log_debug(logger, "Task has %d events more to send for abonent: %s (%s)",
                                       messageToSend.eventCount, messageToSend.abonent.c_str(), 
                                       (bWasReceipted) ? "on receipt":"msg full");
                        isMessageToSend = true; messageToSend.replace = false;
                    } 
                    // if no more events to send => kill task
                    else needKillTask = true;
                }
                else // sent message could be extended and wasn't receipted yet
                {
                    if (task->formatMessage(messageToSend))
                    {
                        messageToSend.replace = true;
                        messageToSend.smsc_id = smsc_id;
                        task->waitResponce(smsc_id);
                        isMessageToSend = true;
                    }
                    else {
                        smsc_log_error(logger, "Failed to extend message #%lld for abonent: %s", 
                                       message.id, message.abonent.c_str());
                        needKillTask = true;
                    }
                }
            }
            else // no events were added to task => set current message wait receipt & kill task
            {
                if (message.isFull() || bWasReceipted) 
                     task->waitReceipt(message.eventCount, smsc_id); // roll current
                else task->waitReceipt(smsc_id);                     // do not roll current
                needKillTask = true;
            }
        }

        if (bWasReceipted) //  process receipt waiting for responce on this smsc_id
        {
            smsc_log_debug(logger, "Processing waiting receipt for smscId=%s.", smsc_id);
            processReceipt(task, receipt.delivered, receipt.retry, smsc_id);
        }
        
        if (needKillTask) taskAccessor.delTask(message.abonent.c_str());
    }
    
    if (isMessageToSend) putToOutQueue(messageToSend);
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry)
{
    const char* smsc_id = smscId.c_str();
    smsc_log_debug(logger, "Receipt: smscId=%s, delivered=%d, retry=%d",
                   smsc_id ? smsc_id:"-", delivered, retry);
    
    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id
    if (smsc_id) smsc_log_debug(logger, "Receipt lock smscId=%s", smsc_id);

    Message message;
    if (Task::getMessage(smsc_id, message)) // in WAIT_RCPT state
    {
        smsc_log_debug(logger, "Receipt found message with smscId=%s.", smsc_id);

        bool isTaskNew = false;
        TaskAccessor taskAccessor(this);
        Task* task = taskAccessor.getTask(message.abonent.c_str(), isTaskNew);
        if (!task) {
            smsc_log_error(logger, "Failed to get task for abonent: %s", message.abonent.c_str());
            return;
        }

        processReceipt(task, delivered, retry, smsc_id, message.id);
        if (isTaskNew) taskAccessor.delTask(message.abonent.c_str());
    }
    else // message by smsc_id not found => wait responce
    {
        smsc_log_debug(logger, "Receipt waiting responce with smscId=%s.", smsc_id);

        if (!responcesTracker.putReceiptData(smsc_id, ReceiptData(delivered, retry)))
            smsc_log_error(logger, "Failed to add receipt data. SMSC id=%lld already used", smsc_id);
    }
}

void TaskProcessor::processReceipt(Task* task, bool delivered, bool retry, 
                                   const char* smsc_id, uint64_t msg_id/*=0*/)
{
    std::string        abonent = task->getAbonent();
    Array<std::string> callers = task->finalizeMessage(smsc_id, delivered, retry, msg_id);
    
    for (int i=0; i<callers.Count(); i++)
    {
        smsc_log_debug(logger, "Event(s) %s to %s from %s",
                       (delivered ? "delivered":(retry ? "expired":"failed")),
                       abonent.c_str(), callers[i].c_str());

        // TODO: send notification(s) to caller(s)
    }

    // do not kill task here !!!
}

/* ---------------------- Technical: receipts & responces waiting & ordering --------------------- */ 

void ResponcesTracker::init(TaskProcessor* _processor, ConfigView* config)
{
    responceWaitTime = parseTime(config->getString("responceWaitTime"));
    if (responceWaitTime <= 0) 
        throw ConfigException("Invalid value for 'responceWaitTime' parameter.");
    receiptWaitTime = parseTime(config->getString("receiptWaitTime"));
    if (receiptWaitTime <= 0) 
        throw ConfigException("Invalid value for 'receiptWaitTime' parameter.");
    
    this->processor = _processor;
}
void ResponcesTracker::Start()
{
    MutexGuard guard(startLock);
    if (!bStarted)
    {
        bNeedExit = false; 
        cleanup();
        Thread::Start();
        bStarted = true;
    }
}
void ResponcesTracker::Stop()
{
    MutexGuard guard(startLock);
    if (bStarted)
    {
        bNeedExit = true;
        {
            MutexGuard guard(responcesMonitor);
            responcesMonitor.notifyAll();
        }
        exitedEvent.Wait();
        cleanup();
        bStarted = false;
    }
}
int ResponcesTracker::Execute()
{
    static const int TRACKER_SERVICE_SLEEP = 1000;
    ResponceTimer respTimer; ReceiptTimer  rcptTimer;
    
    while (!bNeedExit)
    {
        MutexGuard guard(responcesMonitor);
        time_t currentTime = time(NULL);
        time_t nextTime = -1;
        
        while (!bNeedExit && responceWaitQueue.Count() > 0)
        {
            respTimer = responceWaitQueue.Front();
            if (respTimer.timer > currentTime) {
                nextTime = respTimer.timer;
                break;
            }
            else
            {
                responceWaitQueue.Pop(respTimer);
                Message* message = messages.GetPtr(respTimer.seqNum);
                if (message) {
                    smsc_log_warn(logger, "Responce for message #%lld is timed out", message->id);
                    processor->putToOutQueue(*message, true);
                    messages.Delete(respTimer.seqNum);
                }
                else responcesMonitor.wait(TRACKER_SERVICE_SLEEP/100); 
            }
        }

        while (!bNeedExit && receiptWaitQueue.Count() > 0)
        {
            rcptTimer = receiptWaitQueue.Front();
            if (rcptTimer.timer > currentTime) {
                nextTime = (rcptTimer.timer < nextTime) ? rcptTimer.timer : nextTime;
                break;
            }
            else
            {
                receiptWaitQueue.Pop(rcptTimer);
                const char* smsc_id = rcptTimer.smscId.c_str();
                ReceiptData* receipt = receipts.GetPtr(smsc_id);
                if (receipt) {
                    smsc_log_warn(logger, "Responce for receipted smsc_id=%s is timed out", smsc_id);
                    receipts.Delete(smsc_id);
                } 
                else responcesMonitor.wait(TRACKER_SERVICE_SLEEP/100);
            }
        }
        
        if (!bNeedExit) responcesMonitor.wait((nextTime > 0 && nextTime < currentTime) ? 
                                              (currentTime-nextTime)*1000:TRACKER_SERVICE_SLEEP);
    }
    exitedEvent.Signal();
    return 0;
}

void ResponcesTracker::cleanup()
{
    MutexGuard guard(responcesMonitor);
    messages.Empty(); receipts.Empty();
    //responceWaitQueue.Clean(); receiptWaitQueue.Clean();
}
bool ResponcesTracker::putResponceData(int seqNum, const Message& message)
{
    MutexGuard guard(responcesMonitor);
    Message* messagePtr = messages.GetPtr(seqNum);
    if (messagePtr) return false;
    messages.Insert(seqNum, message);
    responceWaitQueue.Push(ResponceTimer(time(NULL)+responceWaitTime, seqNum));
    return true;
}
bool ResponcesTracker::popResponceData(int seqNum, Message& message)
{
    MutexGuard guard(responcesMonitor);
    Message* messagePtr = messages.GetPtr(seqNum);
    if (!messagePtr) return false;
    message = *messagePtr;
    messages.Delete(seqNum);
    return true;
}

bool ResponcesTracker::putReceiptData(const char* smsc_id, const ReceiptData& receipt)            
{
    MutexGuard guard(responcesMonitor);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) return false;
    receipts.Insert(smsc_id, receipt);
    receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smsc_id));
    return true;
}
bool ResponcesTracker::popReceiptData(const char* smsc_id, ReceiptData& receipt)            
{
    MutexGuard guard(responcesMonitor);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (!receiptPtr) return false;
    receipt = *receiptPtr;
    receipts.Delete(smsc_id);
    return true;
}

/* ------------------------ Input (notification) queue access ------------------------ */ 

void TaskProcessor::openInQueue() {
    MutexGuard guard(inQueueMonitor);
    bInQueueOpen = true;
    inQueueMonitor.notifyAll();
}
void TaskProcessor::closeInQueue() {
    MutexGuard guard(inQueueMonitor);
    bInQueueOpen = false;
    inQueueMonitor.notifyAll();
}
bool TaskProcessor::putToInQueue(const MissedCallEvent& event)
{
    MutexGuard guard(inQueueMonitor);
    while (bInQueueOpen && (inQueue.Count() >= maxInQueueSize)) {
        inQueueMonitor.wait();
    }
    if (!bInQueueOpen) return false;
    inQueue.Push(event);
    inQueueMonitor.notifyAll();
    return true;
}
bool TaskProcessor::getFromInQueue(MissedCallEvent& event)
{
    MutexGuard guard(inQueueMonitor);
    do {
        if (inQueue.Count() > 0) {
            inQueue.Pop(event);
            inQueueMonitor.notifyAll();
            return true;
        }
        inQueueMonitor.wait();
    } while (bInQueueOpen);
    smsc_log_info(logger, "Input queue closed.");
    return false;
}

/* ------------------------ Output (sending) queue access ------------------------ */ 

void TaskProcessor::openOutQueue() {
    MutexGuard guard(outQueueMonitor);
    bOutQueueOpen = true;
    outQueueMonitor.notifyAll();
}
void TaskProcessor::closeOutQueue() {
    MutexGuard guard(outQueueMonitor);
    bOutQueueOpen = false;
    outQueueMonitor.notifyAll();
}
bool TaskProcessor::putToOutQueue(const Message& message, bool force/*=false*/)
{
    MutexGuard guard(outQueueMonitor);
    while (!force && bOutQueueOpen && (outQueue.Count() >= maxOutQueueSize)) {
        outQueueMonitor.wait();
    }
    if (!bOutQueueOpen) return false;
    outQueue.Push(message);
    outQueueMonitor.notifyAll();
    return true;
}
bool TaskProcessor::getFromOutQueue(Message& message)
{
    MutexGuard guard(outQueueMonitor);
    do {
        if (outQueue.Count() > 0) {
            outQueue.Pop(message);
            outQueueMonitor.notifyAll();
            return true;
        }
        outQueueMonitor.wait();
    } while (bOutQueueOpen);
    smsc_log_info(logger, "Output queue closed.");
    return false;
}

}}

