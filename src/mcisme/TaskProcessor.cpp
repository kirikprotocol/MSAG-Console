
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
        processor.processResponce(seqNum, delivered, retry, immediate, smscId);
        break;
    case processReceiptMethod:
        processor.processReceipt (smscId, delivered, retry);
        break;
    default:
        __trace2__("Invalid method '%d' invoked by event.", method);
        return -1;
    }
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
        ds(0), dsStatConnection(0), responceWaitTime(0), receiptWaitTime(0), 
        maxInQueueSize(10000), maxOutQueueSize(10000),
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
    
    responceWaitTime = parseTime(config->getString("responceWaitTime"));
    if (responceWaitTime <= 0) 
        throw ConfigException("Invalid value for 'responceWaitTime' parameter.");
    receiptWaitTime = parseTime(config->getString("receiptWaitTime"));
    if (receiptWaitTime <= 0) 
        throw ConfigException("Invalid value for 'receiptWaitTime' parameter.");

    maxInQueueSize  = config->getInt("inputQueueSize");
    maxOutQueueSize = config->getInt("outputQueueSize");

    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("EventsThreadPool"));
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
    this->Stop();
    eventManager.Stop();
    if (mciModule) delete mciModule;

    // TODO: wait & delete all Tasks here !!!
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
        {
            MutexGuard snGuard(messagesBySeqNumLock);
            messagesBySeqNum.Empty();
        }
        
        smsc_log_info(logger, "Loading processing tasks ...");
        tasks = Task::loadupAll();
        smsc_log_info(logger, "Processing tasks loaded.");

        openOutQueue();
        openInQueue();
        Thread::Start();
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
        closeInQueue();
        closeOutQueue();
        exitedEvent.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
}
int TaskProcessor::Execute()
{
    static const char* ERROR_MESSAGE = "Failed to process notification. Details: %s";
    
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
        Task* task = taskAccessor.getTask(abonent, isNewTask);
        if (!task) throw Exception("Failed to obtain task for abonent: %s", abonent);
        task->addEvent(event);
        if (!isNewTask || !task->formatMessage(message)) return;
    }
    putToOutQueue(message);
}
void TaskProcessor::processMessage(const Message& message)
{
    smsc_log_debug(logger, "Sending message #%lld to '%s': %s", 
                   message.id, message.abonent.c_str(), message.message.c_str());

    MutexGuard msGuard(messageSenderLock);
    if (!messageSender) {
        smsc_log_error(logger, "No Message sender defined !!!");
        return;
    }

    int seqNum = messageSender->getSequenceNumber();
    {
        {
            MutexGuard snGuard(messagesBySeqNumLock);
            if (messagesBySeqNum.Exist(seqNum)) {
                smsc_log_warn(logger, "Sequence id=%d was already used !", seqNum);
                messagesBySeqNum.Delete(seqNum);
            }
            messagesBySeqNum.Insert(seqNum, message);
        }
        MutexGuard respGuard(responceWaitQueueLock);
        responceWaitQueue.Push(ResponceTimer(time(NULL)+responceWaitTime, seqNum));
        // Add responce waiting. If timeout will be reached => need resend message (Check thread ???)
    }

    smsc_log_debug(logger, "Sending message #%lld to '%s' ...", message.id, message.abonent.c_str());
    if (!messageSender->send(seqNum, message))
    {
        smsc_log_error(logger, "Failed to send message #%lld to '%s'.", message.id, message.abonent.c_str());
        MutexGuard snGuard(messagesBySeqNumLock);
        if (messagesBySeqNum.Exist(seqNum)) messagesBySeqNum.Delete(seqNum);
        return;
    }
    smsc_log_debug(logger, "Sent message #%lld to '%s'.", message.id, message.abonent.c_str());
}

/* ------------------------ Main processing ------------------------ */ 

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    std::string smscId)
{
    smsc_log_debug(logger, "Responce: seqNum=%d, accepted=%d, retry=%d, immediate=%d",
                   seqNum, accepted, retry, immediate);

    const char* smsc_id = (smscId.length() > 0) ? 0:smscId.c_str();
    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id if exists
    // check accepted here ?
    
    Message message;
    {   
        Message* messagePtr = 0;
        MutexGuard snGuard(messagesBySeqNumLock);
        if (!(messagePtr = messagesBySeqNum.GetPtr(seqNum))) {
            smsc_log_warn(logger, "Unable to locate message for sequence number: %d", seqNum);
            return;
        }
        message = *messagePtr;
        messagesBySeqNum.Delete(seqNum);
    }
    
    bool  needKillTask = false;
    bool  isMessageToSend = false;
    Message messageToSend;
    {
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
                if (task->formatMessage(messageToSend)) isMessageToSend = true;
                else smsc_log_error(logger, "Format message for retry failed for abonent: %s",
                                    message.abonent.c_str());
                
                if (!immediate) messageToSend.attempts++; // TODO: reschedule table in sender
            }
            else        // permanent error
            {
                smsc_log_debug(logger, "Message send error, deleting task for abonent: %s",
                               message.abonent.c_str());
                
                // TODO: line out replace errors in signature & check it here
                // TODO: check REPLACE_FAILED !!!
                
                task->deleteMessages();
                needKillTask = true;
            }
        }
        else    // submit|replace was accepted by SMSC
        {
            if (message.eventCount < task->eventsCount()) // some events were added to task
            {
                if (message.isFull()) // sent message is full
                {
                    // make next current message & skip task events for old message
                    task->waitReceipt(message.eventCount, smsc_id);

                    // try send more events via submit next message 
                    if (task->formatMessage(messageToSend)) { 
                        messageToSend.replace = false;
                        isMessageToSend = true; 
                    } 
                    // if no more events to send => kill task
                    else needKillTask = true;
                }
                else // sent message could be extended
                {
                    if (task->formatMessage(messageToSend))
                    {
                        messageToSend.replace = true;
                        messageToSend.smsc_id = smsc_id;
                        task->waitResponce(smsc_id);
                        isMessageToSend = true;
                    }
                    else {
                        smsc_log_error(logger, "Failed to format message #%lld for abonent: %s", 
                                       message.id, message.abonent.c_str());
                        needKillTask = true;
                    }
                }
            }
            else // no events were added to task
            {
                // set current message wait receipt & kill task
                task->waitReceipt(smsc_id);
                needKillTask = true;
            }
            
            ReceiptData receipt; // check for processed receipt
            if (popReceiptData(smsc_id, receipt))
                processReceipt(task, receipt.delivered, receipt.retry, smsc_id);

            if (needKillTask) taskAccessor.delTask(message.abonent.c_str());
        }
    }
    
    if (isMessageToSend) putToOutQueue(messageToSend);
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry)
{
    const char* smsc_id = smscId.c_str();
    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id

    Message message;
    if (Task::getMessage(smsc_id, message))
    {
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
        if (!putReceiptData(smsc_id, ReceiptData(delivered, retry)))
            smsc_log_error(logger, "Failed to add receipt data. SMSC id=%lld already used", smsc_id);
    }
}

void TaskProcessor::processReceipt(Task* task, bool delivered, bool retry, 
                                   const char* smsc_id, uint64_t msg_id/*=0*/)
{
    // TODO: process receipt on task here

    // 1) send notification(s) to caller(s)
    // 2) delete all events assigned to message
    // 3) delete message
    // 4) update statistics
}

/* ---------------------- Technical: receipts & responces ordering --------------------- */ 

bool TaskProcessor::popReceiptData(const char* smsc_id, ReceiptData& receipt)            
{
    MutexGuard guard(receiptsLock);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (!receiptPtr) return false;
    receipt = *receiptPtr;
    receipts.Delete(smsc_id);
    return true;
}
bool TaskProcessor::putReceiptData(const char* smsc_id, const ReceiptData& receipt)            
{
    MutexGuard guard(receiptsLock);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) return false;
    receipts.Insert(smsc_id, receipt);
    MutexGuard recptGuard(receiptWaitQueueLock);
    receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smsc_id));
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
    while (bInQueueOpen && inQueue.Count() > maxInQueueSize) {
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
bool TaskProcessor::putToOutQueue(const Message& message)
{
    MutexGuard guard(outQueueMonitor);
    while (bOutQueueOpen && outQueue.Count() > maxOutQueueSize) {
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

