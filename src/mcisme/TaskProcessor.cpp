
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
        protocolId(0), svcType(0), address(0), messageSender(0), 
        ds(0), dsStatConnection(0), responceWaitTime(0), receiptWaitTime(0), 
        maxInQueueSize(10000), maxOutQueueSize(10000), // TODO init Queues Size
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

    smsc_log_info(logger, "Load success.");
}
TaskProcessor::~TaskProcessor()
{
    this->Stop();
    eventManager.Stop();
    
    // TODO: wait & delete all Tasks here !!!
    //if (statistics) delete statistics;

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
        //resetWaitingTasks();
        openOutQueue();
        openInQueue();
        Thread::Start();
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

Task* TaskProcessor::createTask(const char* abonent, bool& newone)
{
    // TODO: implement via TaskGuard ???
    newone = false;
    MutexGuard guard(tasksLock);
    Task** taskPtr = tasks.GetPtr(abonent);
    if (taskPtr) return *taskPtr;
    Task* task = new Task(abonent);
    task->loadMessages();
    tasks.Insert(abonent, task);
    newone = true;
    return task;
}
Task* TaskProcessor::getTask(const char* abonent)
{
    // TODO: implement via TaskGuard ???
    MutexGuard guard(tasksLock);
    Task** taskPtr = tasks.GetPtr(abonent);
    return ((taskPtr) ? *taskPtr:0);
}
void TaskProcessor::processEvent(const MissedCallEvent& event)
{
    const char* abonent = event.to.c_str();
    checkAddress(abonent);
    
    Message message;
    {
        bool newone = false;
        Task* task = createTask(abonent, newone);
        
        MutexGuard taskLock(*task); // Lock while operating on task
        task->addEvent(event);
        if (!newone) return;        // Do not send message
        if (!task->getMessage(message)) return;
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

    if (!messageSender->send(seqNum, message))
    {
        smsc_log_error(logger, "Failed to send message #%lld to '%s'", 
                       message.id, message.abonent.c_str());

        MutexGuard snGuard(messagesBySeqNumLock);
        if (messagesBySeqNum.Exist(seqNum)) messagesBySeqNum.Delete(seqNum);
        return;
    }
    smsc_log_debug(logger, "Sent message #%lld to '%s'",
                   message.id, message.abonent.c_str());
}

/* ------------------------ Main processing ------------------------ */ 

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    std::string smscId)
{
    smsc_log_debug(logger, "Responce: seqNum=%d, accepted=%d, retry=%d, immediate=%d",
                   seqNum, accepted, retry, immediate);

    Message message;
    {   
        Message* messagePtr = 0;
        MutexGuard snGuard(messagesBySeqNumLock);
        if (!(messagePtr = messagesBySeqNum.GetPtr(seqNum))) {
            smsc_log_warn(logger, "Failed to locate message. Sequence number=%d is unknown !", seqNum);
            return;
        }
        message = *messagePtr;
        messagesBySeqNum.Delete(seqNum);
    }
    
    Task* task = getTask(message.abonent.c_str()); // ??? or createTask
    if (!task) {
        smsc_log_warn(logger, "Unable to locate task '%s' for sequence number=%d", 
                      message.abonent.c_str(), seqNum);
        // Task MUST be in map for valid responce !!!
        return;
    }
    
    bool  isMessageToSend = false;
    Message messageToSend;
    {
        MutexGuard taskLock(*task); // Lock task while operating on it
    
        if (!accepted)
        {
            if (retry)  // temporal error => keep task & retry with message (possible updated)
            { 
                isMessageToSend = true; 
                if (!task->wasUpdated() || // try to extend current message with new events
                    !task->getMessage(messageToSend)) messageToSend = message; // send old message
                if (!immediate)
                {
                    // TODO: retry with shifted startDelivery time
                    smsc_log_debug(logger, "Need retry with shifted time !");
                }
            }
            else        // permanent error
            {
                // TODO: line out replace errors in signature & check it only

                if (message.replace) // failed to replace smsc_id => try send only new events via submit
                {
                    smsc_log_debug(logger, "Replace message error for abonent:%s smsc_id=%s !",
                                   message.abonent.c_str(), message.smsc_id.c_str());
                    if (task->wasUpdated() && task->getMessage(messageToSend)) {
                        isMessageToSend = true; 
                        messageToSend.replace = false;
                        // TODO: skip old events from message and try send new only
                    }
                    else {
                        // TODO: kill task & delete all messages
                    }
                }
                else
                {
                    // TODO: kill task & delete all messages
                    smsc_log_debug(logger, "Message send error, deleting task for abonent:%s !",
                                   message.abonent.c_str());
                }
            }
        }
        else    // submit|replace was accepted by SMSC
        {
            const char* smsc_id = smscId.c_str();

            ReceiptData receipt;
            checkAddReceipt(smsc_id, receipt);
            if (!receipt.receipted) // wasn't receipted before responce
            {
                if (task->nextMessage(smsc_id, messageToSend)) isMessageToSend = true;
                else {
                    // TODO: kill task
                }
            }
            checkDelReceipt(smsc_id, receipt);
            if (receipt.receipted) // receipt already come
            {
                smsc_log_debug(logger, "Receipt come when responce is in process");
                
                // TODO: process receipt here !!!
            }
        }
    }

    if (isMessageToSend) putToOutQueue(messageToSend);
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry)
{
    const char* smsc_id = smscId.c_str();
    
    if (attachReceipt(smsc_id, delivered, retry)) return;
    
    // TODO: send notification(s) to caller(s)

    if (deleteReceipt(smsc_id)) ; // ???
}

/* ---------------------- Technical: receipts & responces ordering --------------------- */ 

void TaskProcessor::checkDelReceipt(const char* smsc_id, ReceiptData& receipt)
{
    MutexGuard guard(receiptsLock);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) {  
        receipt = *receiptPtr;
        receipts.Delete(smsc_id);
    }
    else receipt.receipted = false;
}
void TaskProcessor::checkAddReceipt(const char* smsc_id, ReceiptData& receipt)
{
    MutexGuard guard(receiptsLock);
    receipt.receipted = false;
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) receipt = *receiptPtr;
    else {
        receipts.Insert(smsc_id, receipt);
        MutexGuard recptGuard(receiptWaitQueueLock);
        receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smsc_id));
    }
}
bool TaskProcessor::deleteReceipt(const char* smsc_id)
{
    MutexGuard guard(receiptsLock);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) {  
        receipts.Delete(smsc_id);
        return true;
    }
    return false;
}
bool TaskProcessor::attachReceipt(const char* smsc_id, bool delivered, bool retry)
{
    MutexGuard guard(receiptsLock);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) {  // attach receipt data;
        receiptPtr->receipted = true;
        receiptPtr->delivered = delivered;
        receiptPtr->retry     = retry;
        return true;
    }
    else {
        receipts.Insert(smsc_id, ReceiptData(true, delivered, retry));
        MutexGuard recptGuard(receiptWaitQueueLock);
        receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smsc_id));
    }
    return false;
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

