
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
    try 
    {
        std::auto_ptr<char> dsIdentity(config->getString("type"));
        const char* dsIdentityStr = dsIdentity.get();
        ds = DataSourceFactory::getDataSource(dsIdentityStr);
        if (!ds) throw ConfigException("DataSource for '%s' identity "
                                       "wasn't registered !", dsIdentityStr);
        ds->init(config);
    }
    catch (ConfigException& exc)
    {
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
        bStarted(false), bNeedExit(false), bInQueueOpen(false), bOutQueueOpen(false)
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
    smsc_log_info(logger, "Load success.");
}
TaskProcessor::~TaskProcessor()
{
    this->Stop();
    eventManager.Stop();
    
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
            MutexGuard snGuard(taskIdsBySeqNumLock);
            taskIdsBySeqNum.Empty();
        }
        //resetWaitingTasks();
        bNeedExit = false;
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
        bNeedExit = true;
        closeInQueue();
        closeOutQueue();
        exitedEvent.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
}
int TaskProcessor::Execute()
{
    while (!bNeedExit)
    {
        MissedCallEvent event;
        if (!getFromInQueue(event)) break;
        
        // TODO: process event
    }
    exitedEvent.Signal();
    return 0;
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
    return false;
}

/* ------------------------ Main processing ------------------------ */ 

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    std::string smscId)
{
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry)
{
}

}}

