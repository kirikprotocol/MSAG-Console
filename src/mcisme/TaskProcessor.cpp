
#include <exception>

#include "TaskProcessor.h"

extern bool isMSISDNAddress(const char* string);
extern "C" void clearSignalMask(void);

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
    clearSignalMask();

    switch (method)
    {
    case processResponceMethod:
        processor.processResponce(seqNum, delivered, retry, immediate,
                                  cancel, cancel_failed, smscId);
        break;
    case processReceiptMethod:
        processor.processReceipt (smscId, delivered, expired, deleted);
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
        if (!ds) throw ConfigException("DataSource for '%s' identity wasn't registered !", dsIdentityStr);
        ds->init(config);
    } 
    catch (ConfigException& exc) {
        if (ds) delete ds; ds = 0;
        throw;
    }
}

TaskProcessor::TaskProcessor(ConfigView* config)
    : Thread(), MissedCallListener(), AdminInterface(), 
        logger(Logger::getInstance("smsc.mcisme.TaskProcessor")), 
        protocolId(0), daysValid(1), svcType(0), address(0), templateManager(0), mciModule(0), messageSender(0),
        ds(0), dsStatConnection(0), statistics(0), maxInQueueSize(10000), maxOutQueueSize(10000),
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
    try { daysValid = config->getInt("DaysValid"); }
    catch(ConfigException& exc) { daysValid = 1; };

    std::string callingMask = config->getString("CallingMask");
    std::string calledMask = config->getString("CalledMask");
    
    smsc_log_info(logger, "Loading templates ...");
    std::auto_ptr<ConfigView> templatesCfgGuard(config->getSubConfig("Templates"));
    ConfigView* templatesCfg = templatesCfgGuard.get();
    templateManager = new TemplateManager(templatesCfg);
    smsc_log_info(logger, "Templates loaded.");

    std::auto_ptr<ConfigView> circuitsCfgGuard(config->getSubConfig("Circuits"));
    ConfigView* circuitsCfg = circuitsCfgGuard.get();
    Circuits circuits;
    circuits.hsn = circuitsCfg->getInt("hsn");
    circuits.spn = circuitsCfg->getInt("spn");
    const char* tsmStr = circuitsCfg->getString("tsm");
    uint32_t tsmLong = 0;
    if (!tsmStr || !tsmStr[0] || sscanf(tsmStr, "%lx", &tsmLong) != 1)
        throw ConfigException("Parameter <MCISme.Circuits.tsm> value is empty or invalid."
                              " Expecting hex string, found '%s'.", tsmStr ? tsmStr:"null");
    //smsc_log_debug(logger, "TSM = %lx (%ld)", tsmLong, tsmLong);
    circuits.ts = tsmLong;
    
    std::auto_ptr<ConfigView> releaseSettingsCfgGuard(config->getSubConfig("Reasons"));
    ConfigView* releaseSettingsCfg = releaseSettingsCfgGuard.get();
    ReleaseSettings releaseSettings;
    releaseSettings.busyCause           = releaseSettingsCfg->getInt ("Busy.cause");
    releaseSettings.busyInform          = releaseSettingsCfg->getBool("Busy.inform") ? 1:0;
    releaseSettings.noReplyCause        = releaseSettingsCfg->getInt ("NoReply.cause");
    releaseSettings.noReplyInform       = releaseSettingsCfg->getBool("NoReply.inform") ? 1:0;
    releaseSettings.unconditionalCause  = releaseSettingsCfg->getInt ("Unconditional.cause");
    releaseSettings.unconditionalInform = releaseSettingsCfg->getBool("Unconditional.inform") ? 1:0;
    releaseSettings.absentCause         = releaseSettingsCfg->getInt ("Absent.cause");
    releaseSettings.absentInform        = releaseSettingsCfg->getBool("Absent.inform") ? 1:0;
    releaseSettings.otherCause          = releaseSettingsCfg->getInt ("Other.cause");
    releaseSettings.otherInform         = releaseSettingsCfg->getBool("Other.inform") ? 1:0;
    
    mciModule = new MCIModule(circuits, releaseSettings, 
                              callingMask.c_str(), calledMask.c_str());
    
    responcesTracker.init(this, config);

    maxInQueueSize  = config->getInt("inputQueueSize");
    maxOutQueueSize = config->getInt("outputQueueSize");

    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("SMPPThreadPool"));
    eventManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    try { Task::bInformAll = config->getBool("forceInform"); } catch (...) { Task::bInformAll = false;
        smsc_log_warn(logger, "Parameter <MCISme.forceInform> missed. Force inform for all abonents is off");
    }
    try { Task::bNotifyAll = config->getBool("forceNotify"); } catch (...) { Task::bNotifyAll = false;
        smsc_log_warn(logger, "Parameter <MCISme.forceNotify> missed. Force notify for all abonents is off");
    }

    int rowsPerMessage = 5;
    try { rowsPerMessage = config->getInt("maxRowsPerMessage"); } catch (...) { rowsPerMessage = 5;
        smsc_log_warn(logger, "Parameter <MCISme.maxRowsPerMessage> is invalid. Using default %d", rowsPerMessage);
    }
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("DataSource"));
    initDataSource(dsIntCfgGuard.get());
    
    dsStatConnection = ds->getConnection();
    statistics = new StatisticsManager(dsStatConnection);
    if (statistics) statistics->Start();
    
    AbonentProfiler::init(ds);
    Task::init(ds, statistics, rowsPerMessage);
    
    smsc_log_info(logger, "Load success.");
}
TaskProcessor::~TaskProcessor()
{
    eventManager.Stop();
    this->Stop();
    
    if (templateManager) delete templateManager;
    if (mciModule) delete mciModule;
    if (statistics) delete statistics;
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
        if (mciModule) mciModule->Attach(this);
        
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
        
        if (mciModule) mciModule->Detach();
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
    clearSignalMask();
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

/* Is calling on Startup. Initiates current messages sending for all tasks.
   task->getCurrentState() == (W_RESP, W_RCPT, W_CNCL);
   1) W_CNCL || W_RCPT => task->getCurrentSmscId() & try cancel
   2) W_RESP => format new message & send */
void TaskProcessor::loadupTasks() 
{
    MutexGuard guard(tasksMonitor);
    smsc_log_info(logger, "Loading processing tasks ...");
    lockedTasks.Empty();
    tasks = Task::loadupAll();
    smsc_log_info(logger, "Processing tasks loaded. Resending messages...");
    
    Array<std::string> abonentsToSkip(10);

    Message message;
    char* abonent=0; Task* task = 0; tasks.First();
    while (tasks.Next(abonent, task))
    {
        if (!abonent || !task) continue;

        MessageState state = task->getCurrentState();
        int allEventsCount = task->getEventsCount();
        int newEventsCount = task->getNewEventsCount();
        smsc_log_debug(logger, "Loadup: task for abonent %s loaded (state=%d). Events: all=%d, new=%d", 
                       abonent, (int)state, allEventsCount, newEventsCount);
        {
            AbonentProfile profile = task->getAbonentProfile();
            task->setTemplateFormatter((templateManager) ? 
                                        templateManager->getInformFormatter(profile.informTemplateId):0);
        }
        
        if (state == WAIT_CNCL || (state == WAIT_RCPT && newEventsCount > 0))
        {
            message.reset(abonent); message.cancel = true;
            message.id = task->getCurrentMessageId();
            message.smsc_id = task->getCurrentSmscId();
            const char* smsc_id = message.smsc_id.c_str();
            if (message.id <= 0) {
                smsc_log_warn(logger, "Loadup: current message id is undefined for abonent %s", abonent);
                continue;
            }
            if (!smsc_id || !smsc_id[0]) {
                smsc_log_warn(logger, "Loadup: smscId is undefined for message #%lld", message.id);
                continue;
            }
            task->waitCancel(smsc_id);
            smsc_log_debug(logger, "Loadup: cancel message #%lld smscId=%s put to out queue (state=%d)",
                           message.id, smsc_id, (int)state);
            putToOutQueue(message, true);
        }
        else if (state != WAIT_RCPT && task->formatMessage(message))
        {
            smsc_log_debug(logger, "Loadup: new message %lld put to out queue (state=%d). Events=%d Rows=%d",
                           message.id, (int)state, message.eventsCount, message.rowsCount);
            if (state != WAIT_RESP) task->waitResponce();
            putToOutQueue(message, true);
        }
        else
        {
            smsc_log_warn(logger, "Loadup: task for abonent %s has no events to send (state=%d). Skipped",
                          abonent, (int)state);
            abonentsToSkip.Push(abonent);
        }
    }

    // Delete skipped tasks
    for (int i=0; i<abonentsToSkip.Count(); i++) {
        const char* abonent = abonentsToSkip[i].c_str();
        if (abonent && abonent[0]) tasks.Delete(abonent);
    }

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
    try 
    { 
        task->loadup();
        AbonentProfile profile = task->getAbonentProfile();
        task->setTemplateFormatter((templateManager) ? 
                                    templateManager->getInformFormatter(profile.informTemplateId):0);
    } catch (std::exception& exc) { 
        smsc_log_error(logger, "Task loadup for abonent %s failed", abonent);
        delete task; throw;
    } catch (...) { 
        delete task; throw;
    }
    
    lockedTasks.Insert(abonent, task);
    tasks.Insert(abonent, task);
    newone = true;

    smsc_log_debug(logger, "Task for abonent %s created (state=%d). Events=%d",
                   abonent, (int)(task->getCurrentState()), task->getEventsCount());

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
    
    smsc_log_debug(logger, "Task for abonent %s killed (state=%d). Events=%d", 
                   abonent, (int)((*taskPtr)->getCurrentState()), (*taskPtr)->getEventsCount());

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

inline static bool checkEventMask(uint8_t userMask, uint8_t eventCause)
{
    return ((userMask & eventCause) == eventCause);
}
void TaskProcessor::processEvent(const MissedCallEvent& event)
{
    const char* abonent = event.to.c_str();
    checkAddress(abonent);
    
    AbonentProfile profile = AbonentProfiler::getProfile(abonent);
    if (!checkEventMask(profile.eventMask, event.cause)) {
        smsc_log_debug(logger, "Event: for abonent %s skipped (userMask=%02X, eventCause=%02X).",
                       abonent, profile.eventMask, event.cause);
        return; // skip event if user mask not permit it
    }

    Message message; bool needMessage = false;
    if (profile.inform || Task::bInformAll)
    {
        TaskAccessor taskAccessor(this);

        bool isNewTask = false; // get or create task
        Task* task = taskAccessor.getTask(abonent, isNewTask); 
        if (!task) throw Exception("Event: failed to obtain task for abonent %s", abonent);

        task->addEvent(event); // add new event to task chain (inassigned to message in DB)
        MessageState state = task->getCurrentState();
        smsc_log_debug(logger, "Event: for %s added to %s task (state=%d). Events=%d",
                       abonent, (isNewTask) ? "new":"existed", (int)state, task->getEventsCount());

        if (!isNewTask && state != WAIT_RCPT) { 
            smsc_log_debug(logger, "Event: is put off. Task for abonent %s "
                           "is already processing events (state=%d)", abonent, (int)state);
            return; // process event only if task not exists or task is awaiting receipt
        } 

        message.reset(abonent);
        message.smsc_id = task->getCurrentSmscId();
        const char* smsc_id = (message.smsc_id.length() > 0) ? message.smsc_id.c_str():0;

        if (state == WAIT_RCPT && smsc_id && smsc_id[0])
        {
            message.id = task->getCurrentMessageId(); 
            if (message.id > 0) { // task is waiting receipt for incomplete message
                smsc_log_debug(logger, "Event: message #%lld (smscId=%s) for abonent %s cancelling (extending)",
                               message.id, (smsc_id) ? smsc_id:"-", abonent);
                message.cancel = true; needMessage = true;
                task->waitCancel(smsc_id);
            } 
            else smsc_log_error(logger, "Event: current message id is invalid for abonent %s", abonent);
        } 
        else
        {
            if (task->formatMessage(message)) {
                smsc_log_debug(logger, "Event: submitting new message #%lld, for abonent %s",
                               message.id, abonent);
                message.cancel = false; needMessage = true;
                task->waitResponce();
            }
            else smsc_log_error(logger, "Event: failed to format new message for abonent %s", abonent);
        }
    } 
    else smsc_log_debug(logger, "Event: for abonent %s skipped (inform flag is off).", abonent);

    if (needMessage) putToOutQueue(message);
}

void TaskProcessor::processMessage(const Message& message)
{
    smsc_log_debug(logger, "Sending %s message #%lld to '%s'",
                   ((message.notification) ? "notify":((message.cancel) ? "cancel":"submit")),
                   message.id, message.abonent.c_str());

    MutexGuard msGuard(messageSenderLock);
    if (!messageSender) {
        smsc_log_error(logger, "No Message sender defined !!!");
        return;
    }

    int seqNum = messageSender->getSequenceNumber();
    if (!responcesTracker.putResponceData(seqNum, message)) {
        smsc_log_error(logger, "Sequence id=%d was already used!", seqNum);
        return;
    }

    if (!messageSender->send(seqNum, message)) {
        smsc_log_error(logger, "Failed to send %s message #%lld for abonent %s",
                       ((message.notification) ? "notify":((message.cancel) ? "cancel":"submit")),
                       message.id, message.abonent.c_str());
        invokeProcessResponce(seqNum, false, false, false, message.cancel, false, message.smsc_id);
        return;
    }
    
    smsc_log_debug(logger, "Message #%lld for abonent %s. %s sent", message.id, message.abonent.c_str(), 
                   ((message.notification) ? "Notify":((message.cancel) ? "Cancel":"Submit")));
}

/* ------------------------ Main processing ------------------------ */ 

void TaskProcessor::processNotificationResponce(Message& message, 
                                                bool accepted, bool retry, bool immediate, std::string smscId)
{
    const char* abonent = message.abonent.c_str();
    const char* smsc_id = (smscId.length() > 0) ? smscId.c_str():0;
    
    smsc_log_debug(logger, "Got notification responce: smscId=%s, accepted=%d, retry=%d, immediate=%d",
                   smsc_id ? smsc_id:"-", (int)accepted, (int)retry, (int)immediate);
    
    ReceiptData receipt; // check waiting receipt existance
    if (responcesTracker.popReceiptData(smsc_id, receipt))
        smsc_log_warn(logger, "Got receipt for notification message with smscId=%s", smsc_id ? smsc_id:"-");
    
    if (!accepted) 
    {
        if (retry) {
            smsc_log_debug(logger, "Retrying to send notification message to abonent %s", abonent);
            if (!immediate) message.attempts++;
            putToOutQueue(message);
        }
        else { // permanent error
            smsc_log_error(logger, "Failed to send notification message to abonent %s", abonent);
        }
    }
    else // accepted
    { 
        smsc_log_debug(logger, "Succeeded to send notification message to abonent %s", abonent);
        if(statistics) statistics->incNotified();
    }
}

bool TaskProcessor::formatMessage(Task* task, Message& message)
{
    __require__(task); 

    if (task->formatMessage(message)) {
        smsc_log_debug(logger, "Need send %d events more in message #%lld for abonent %s", 
                       message.eventsCount, message.id, task->getAbonent().c_str());
        message.cancel = false;
        task->waitResponce();
        return true;
    }
    return false;
}
bool TaskProcessor::processCancel(Task* task, const char* smsc_id, Message& message,
                                  bool receipted/*=false*/, bool delivered/*=false*/, bool expired/*=false*/)
{
    __require__(task); 
    __require__(smsc_id);
    __require__(task->getCurrentState() == WAIT_CNCL);

    bool result = false;
    const char* abonent = task->getAbonent().c_str();

    uint64_t msg_id = task->getCurrentMessageId();
    int newEvtCount = task->getNewEventsCount(); if (newEvtCount < 0) newEvtCount = 0; 
    int allEvtCount = task->getEventsCount();    if (allEvtCount < 0) allEvtCount = 0;
    
    smsc_log_debug(logger, "Processing cancel on smscId=%s (all=%d new=%d) for abonent %s",
                   smsc_id, allEvtCount, newEvtCount, abonent);
    
    // set old message WAIT_RCPT & skip task events for it, clear current message
    int oldEvtCount = allEvtCount - newEvtCount; if (oldEvtCount < 0) oldEvtCount = 0;
    task->waitReceipt(oldEvtCount, smsc_id);
    
    // process finalization for old message (message is in WAIT_CNCL with smsc_id)
    if (receipted) processReceipt(task, delivered, expired, smsc_id, msg_id);
    
    // try send more events via submit new message (do not cancel)
    return (newEvtCount > 0) ? formatMessage(task, message):false;
}

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    bool cancel, bool cancel_failed, std::string smscId)
{
    smsc_log_debug(logger, "Responce (%s): seqNum=%d, smscId=%s, accepted=%d, retry=%d, immediate=%d, cancel_failed=%d",
                   cancel ? "cancel":"submit", seqNum, smscId.c_str(), 
                   (int)accepted, (int)retry, (int)immediate, (int)cancel_failed);

    Message message; // get message by sequence number assigned in sender
    if (!responcesTracker.popResponceData(seqNum, message)) {
        if (!cancel) smsc_log_error(logger, "Responce: unable to locate message for seqNum=%d", seqNum);
        else smsc_log_debug(logger, "Responce: cancel message for seqNum=%d was processed by receipt", seqNum);
        return;
    }
    if (!message.notification && message.cancel != cancel) {
        smsc_log_error(logger, "Responce: %s got on %s message #%lld, seqNum=%d",
                       cancel ? "cancel":"submit", message.cancel ? "cancel":"submit", message.id, seqNum);
        return;
    }

    if (cancel) smscId=message.smsc_id;
    const char* smsc_id = (smscId.length() > 0) ? smscId.c_str():0;
    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id if exists
    if (smsc_id) smsc_log_debug(logger, "Responce lock smscId=%s", smsc_id);
    
    if (message.notification) {
        processNotificationResponce(message, accepted, retry, immediate, smscId);
        return;
    }
    
    ReceiptData receipt; // check waiting receipt existance
    bool bWasReceipted = responcesTracker.popReceiptData(smsc_id, receipt);
    
    // skip responce processing if cancel was already processed by receipt
    if (bWasReceipted && (cancel || cancel_failed)) { 
        smsc_log_debug(logger, "Responce: cancel was receipted & already processed. "
                       "Message #%lld (smscId=%s), abonent %s", message.id, smsc_id, message.abonent.c_str());
        return;
    }
    
    bool  needKillTask = false;
    bool  isMessageToSend = false;
    Message messageToSend;
    {
        TaskAccessor taskAccessor(this);
        Task* task = taskAccessor.getTask(message.abonent.c_str());
        if (!task) { // Do not create task! Task MUST be in map for valid responce.
            smsc_log_error(logger, "Unable to locate task for abonent %s", message.abonent.c_str());
            return;
        }
        MessageState state = task->getCurrentState();

        if (state == WAIT_CNCL) // task is waiting responce on cancel message
        {
            if (!cancel || !message.cancel || !smsc_id || !smsc_id[0]) {
                smsc_log_error(logger, "Canceling task for abonent %s got invalid responce (c=%d, mc=%d, smscId=%s)",
                               message.abonent.c_str(), (int)cancel, (int)message.cancel, (smsc_id) ? smsc_id:"-");
                return;
            }

            if (accepted) // cancel ok
            {
                smsc_log_debug(logger, "Responce: cancel ok. Message #%lld (smscId=%s), abonent %s",
                               message.id, smsc_id, message.abonent.c_str());
                // mark receipt to quit
                if (!responcesTracker.putReceiptData(smsc_id, ReceiptData(false, false)))
                    smsc_log_warn(logger, "Failed to add receipt data (on responce). smscId=%s already used", smsc_id);
                else smsc_log_debug(logger, "Responce: cancel ok processed (smscId=%s) & mark receipt to quit", smsc_id);

                // try send more events via submit new message
                isMessageToSend = formatMessage(task, messageToSend);
                if (!isMessageToSend) {
                    smsc_log_warn(logger, "Responce: no events to send after cancel ok (smscId=%s) for abonent %s",
                                  smsc_id, message.abonent.c_str());
                    needKillTask = true;
                }
            }
            else // cancel error
            {
                if (cancel_failed) // permanent error
                { 
                    smsc_log_debug(logger, "Responce: cancel failed. Message #%lld (smscId=%s), abonent %s",
                                   message.id, smsc_id, message.abonent.c_str()); 

                    // will roll current message for old event(s) to WAIT_RCPT & try to format new message
                    isMessageToSend = processCancel(task, smsc_id, messageToSend);
                    needKillTask = !isMessageToSend;
                }
                else if (retry)  // temporal error => keep task & resend cancel message
                { 
                    isMessageToSend = true; messageToSend = message; 
                    messageToSend.cancel = true; messageToSend.notification = false;
                    if (!immediate) messageToSend.attempts++;
                } 
                else {
                    smsc_log_error(logger, "Canceling task for abonent %s got invalid responce (r=%d, cf=%d, smscId=%s)",
                                   message.abonent.c_str(), (int)retry, (int)cancel_failed, (int)smsc_id); 
                    return;
                }
            }
        }
        else if (state == WAIT_RESP) // task is waiting responce on submit message
        {
            if (!accepted)
            {
                if (retry) // resend current message
                {
                    isMessageToSend = true; messageToSend = message;
                    messageToSend.cancel = false; messageToSend.notification = false;
                    if (!immediate) messageToSend.attempts++;
                }
                else      // permanent error
                {
                    smsc_log_debug(logger, "Responce: message #%lld (smscId=%s) submit error for abonent %s",
                                   message.id, smsc_id ? smsc_id:"-", message.abonent.c_str());
                    if (bWasReceipted) {
                        smsc_log_warn(logger, "Responce: message #%lld (smscId=%s) was receipted "
                                      "after submit error for abonent %s", message.id,
                                      smsc_id ? smsc_id:"-", message.abonent.c_str());
                        bWasReceipted = false;
                    }
                    task->finalizeMessage(smsc_id, false, false, message.id);
                    needKillTask = true;
                }
            }
            else // usual (submit) message was accepted by SMSC
            {
                if (!smsc_id) {
                    smsc_log_error(logger, "Responce: got invalid submit ok responce (smscId=-) on message #%lld",
                                   message.id);
                    return;
                }
                smsc_log_debug(logger, "Responce: message #%lld (smscId=%s) submit ok for abonent %s",
                               message.id, smsc_id, message.abonent.c_str());
                
                if (message.eventsCount < task->getEventsCount()) // extra events were added to task
                {
                    if (message.isFull() || bWasReceipted) // sent message was full or was receipted
                    {
                        // set sent message WAIT_RCPT & skip task events for it, set clear current message
                        task->waitReceipt(message.eventsCount, smsc_id);
                        // try send more events via submit new message
                        isMessageToSend = formatMessage(task, messageToSend);
                        if (!isMessageToSend) {
                            smsc_log_debug(logger, "Responce: no events to send after submit ok (smscId=%s) for abonent %s",
                                           smsc_id, message.abonent.c_str());
                            needKillTask = true; // if no more events to send => kill task
                        }
                    }
                    else // sent message could be extended and wasn't receipted yet => send cancel
                    {
                        task->waitCancel(smsc_id);
                        isMessageToSend = true; messageToSend.reset(message.abonent);
                        messageToSend.id = message.id; messageToSend.smsc_id = smsc_id;
                        messageToSend.cancel = true; messageToSend.notification = false;
                    }
                }
                else // no events were added to task => set current message wait receipt & kill task
                {
                    if (message.isFull() || bWasReceipted) 
                         task->waitReceipt(message.eventsCount, smsc_id); // roll current
                    else task->waitReceipt(smsc_id);                      // do not roll current
                    needKillTask = true;
                }
            }

            if (bWasReceipted && smsc_id) { //  process receipt waiting for responce on this smsc_id
                smsc_log_debug(logger, "Responce: processing waiting receipt for smscId=%s", smsc_id);
                processReceipt(task, receipt.delivered, receipt.expired, smsc_id);
            }
        }
        else
        {
            smsc_log_error(logger, "Responce: task for abonent %s is in invalid state=%d for %s responce",
                           message.abonent.c_str(), (int)state, cancel ? "cancel":"submit");
            return;
        }
        
        if (needKillTask) taskAccessor.delTask(message.abonent.c_str());
    }
    
    if (isMessageToSend) putToOutQueue(messageToSend);
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool expired, bool deleted)
{
    const char* smsc_id = (smscId.length() > 0) ? smscId.c_str():0;
    if (!smsc_id || !smsc_id[0]) {
        smsc_log_error(logger, "Invalid receipt came (smscId=-)");
        return;
    }
    smsc_log_debug(logger, "Receipt: smscId=%s (delivered=%d, expired=%d, deleted=%d)",
                   smsc_id, (int)delivered, (int)expired, (int)deleted);
    
    const char* abonent = 0;
    bool isMessageToSend = false;
    Message message; Message messageToSend;
    
    // try get cancel message by smsc_id (will skip responce processing)
    bool cancelResponceUnprocessed = responcesTracker.popResponceData(smsc_id, message);

    SmscIdAccessor smscIdAccessor(this, smsc_id); // lock smsc_id
    smsc_log_debug(logger, "Receipt: lock smscId=%s", smsc_id);
    
    if (cancelResponceUnprocessed) // task is waiting cancel for old message
    {
        abonent = message.abonent.c_str();
        smsc_log_debug(logger, "Receipt: got cancel message #%lld (smscId=%s) for abonent %s",
                       message.id, smsc_id, abonent);
        
        TaskAccessor taskAccessor(this);
        Task* task = taskAccessor.getTask(abonent); // do not create task! Task MUST be in map for valid responce.
        if (!task) {
            smsc_log_error(logger, "Receipt: failed to obtain task for abonent %s", abonent);
            return;
        }
        MessageState state = task->getCurrentState();
        if (state != WAIT_CNCL) {
            smsc_log_error(logger, "Receipt: task for abonent %s has invalid state=%d (expecting WAIT_CNCL)",
                           abonent, (int)state);
            return;
        }
        isMessageToSend = (deleted) ? formatMessage(task, messageToSend):
                          processCancel(task, smsc_id, messageToSend, true, delivered, expired);
        if (!isMessageToSend) {
            if (deleted) smsc_log_warn(logger, "Receipt: no events to send after cancel ok receipt "
                                       "(smscId=%s) for abonent %s", smsc_id, abonent);
            else smsc_log_debug(logger, "Receipt: no events to send after cancel fail receipt "
                                "(smscId=%s) for abonent %s", smsc_id, abonent);
            taskAccessor.delTask(abonent); // if no more events to send => kill task
        }
    }
    else // processing usual receipt or cancel responce processing (or was already processed)
    {
        ReceiptData receipt;
        if (responcesTracker.popReceiptData(smsc_id, receipt)) { // responce has marked smsc_id to quit
            smsc_log_debug(logger, "Receipt: smscId=%s was already processed (cancel or notify)", smsc_id);
            return;
        }

        MessageState state = UNKNOWNST;
        if (Task::getMessage(smsc_id, message, state)) // got message by smsc_id
        {
            abonent = message.abonent.c_str();
            smsc_log_debug(logger, "Receipt: found message #%lld (smscId=%s) for abonent %s, state=%d",
                           message.id, smsc_id, abonent, (int)state);
            
            bool isNewTask = false;
            TaskAccessor taskAccessor(this);
            Task* task = taskAccessor.getTask(abonent, isNewTask);
            if (!task) {
                smsc_log_error(logger, "Receipt: failed to obtain/create task for abonent %s", abonent);
                return;
            }
            
            // Responce is processing (possible locked on SmscIdAccessor or TaskAccessor)
            if (!isNewTask && message.cancel && task->getCurrentMessageId() == message.id &&
                task->getCurrentState() == WAIT_CNCL) 
            {
                isMessageToSend = (deleted) ? formatMessage(task, messageToSend):
                                  processCancel(task, smsc_id, messageToSend, true, delivered, expired);
                if (!isMessageToSend) {
                    smsc_log_warn(logger, "No events to send after cancel %s receipt (smscId=%s) for abonent %s",
                                  (deleted) ? "ok":"fail", smsc_id, abonent);
                    //taskAccessor.delTask(abonent); // if no more events to send => kill task
                }
                if (!responcesTracker.putReceiptData(smsc_id, ReceiptData(delivered, expired)))
                    smsc_log_warn(logger, "Failed to add receipt data (on receipt 2). smscId=%s already used", smsc_id);
                else smsc_log_debug(logger, "Receipt has processed (smscId=%s) & mark responce to quit", smsc_id);
            }
            else
            {
                processReceipt(task, delivered, expired, smsc_id, message.id);
                if (isNewTask) taskAccessor.delTask(abonent);
            }
        }
        else // message by smsc_id not found => wait SMSC responce with smsc_id
        {
            smsc_log_debug(logger, "Receipt: processing (smscId=%s) put off till responce come", smsc_id);
            if (!responcesTracker.putReceiptData(smsc_id, ReceiptData(delivered, expired)))
                smsc_log_warn(logger, "Failed to add receipt data (on receipt 1). smscId=%s already used", smsc_id);
        }
    }

    if (isMessageToSend) putToOutQueue(messageToSend);
}

void TaskProcessor::processReceipt(Task* task, bool delivered, bool expired, 
                                   const char* smsc_id, uint64_t msg_id/*=0*/)
{
    std::string        abonent = task->getAbonent();
    Array<std::string> callers = task->finalizeMessage(smsc_id, delivered, expired, msg_id);
    if (callers.Count() <= 0) return;

    AbonentProfile profile = task->getAbonentProfile();
    if (!profile.notify && !Task::bNotifyAll) return;
    NotifyTemplateFormatter* notifyFormatter = (templateManager) ? 
        templateManager->getNotifyFormatter(profile.notifyTemplateId):0;
    OutputFormatter* formatter = (notifyFormatter) ? notifyFormatter->getMessageFormatter():0;
    if (!formatter) {
        smsc_log_error(logger, "Failed to locate notify template formatter for abonent %s", abonent.c_str());
        return;
    }
    
    for (int i=0; i<callers.Count(); i++)
    {
        smsc_log_debug(logger, "Event(s) %s to %s from %s", 
                       (delivered ? "delivered":(expired ? "expired":"failed")), abonent.c_str(), callers[i].c_str());
        
        if (!delivered || callers[i].length() <= 0) continue;

        Message message; message.abonent = callers[i];
        message.cancel=false; message.notification = true;
        try 
        {
            ContextEnvironment ctx;
            NotifyGetAdapter adapter(abonent, callers[i]);
            formatter->format(message.message, adapter, ctx);
        } 
        catch (std::exception& exc) {
            smsc_log_error(logger, "Failed to format notify message for abonent %s. Details: %s",
                           abonent.c_str(), exc.what());
        }
        catch (...) {
            smsc_log_error(logger, "Failed to format notify message for abonent %s. Reason is unknown",
                           abonent.c_str());
        }
        putToOutQueue(message);
    }
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
    clearSignalMask();

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
                    const char* smsc_id = message->smsc_id.c_str();
                    smsc_log_warn(logger, "Responce for %s message #%lld smscId=%s is timed out. Resending...",
                                  (message->cancel) ? "cancel":"submit", message->id, smsc_id ? smsc_id:"-");
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
                const char* smsc_id = (rcptTimer.smscId.length() > 0) ? rcptTimer.smscId.c_str():0;
                ReceiptData* receipt = (smsc_id) ? receipts.GetPtr(smsc_id):0;
                if (receipt) {
                    smsc_log_debug(logger, "Responce for receipted smscId=%s is timed out. Deleting",
                                   smsc_id ? smsc_id:"-");
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
    smscIds.Empty(); messages.Empty(); receipts.Empty();
    //responceWaitQueue.Clean(); receiptWaitQueue.Clean();
}
bool ResponcesTracker::putResponceData(int seqNum, const Message& message)
{
    MutexGuard guard(responcesMonitor);
    const char* smsc_id = (message.cancel) ? message.smsc_id.c_str():0;
    Message* messagePtr = messages.GetPtr(seqNum);
    if (messagePtr) return false;
    if (smsc_id && smsc_id[0] && !smscIds.Exists(smsc_id)) smscIds.Insert(smsc_id, seqNum);
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
    const char* smsc_id = (message.cancel) ? message.smsc_id.c_str():0;
    if (smsc_id && smsc_id[0] && smscIds.Exists(smsc_id)) smscIds.Delete(smsc_id);
    return true;
}
bool ResponcesTracker::popResponceData(const char* smsc_id, Message& message)
{
    MutexGuard guard(responcesMonitor);
    if (!smsc_id || !smsc_id[0]) return false;
    int* seqNum = smscIds.GetPtr(smsc_id);
    if (!seqNum) return false;
    Message* messagePtr = messages.GetPtr(*seqNum);
    if (!messagePtr) return false;
    message = *messagePtr;
    messages.Delete(*seqNum);
    smscIds.Delete(smsc_id);
    return true;
}
bool ResponcesTracker::putReceiptData(const char* smsc_id, const ReceiptData& receipt)            
{
    if (!smsc_id || !smsc_id[0]) return false;
    MutexGuard guard(responcesMonitor);
    ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
    if (receiptPtr) return false;
    receipts.Insert(smsc_id, receipt);
    receiptWaitQueue.Push(ReceiptTimer(time(NULL)+receiptWaitTime, smsc_id));
    return true;
}
bool ResponcesTracker::popReceiptData(const char* smsc_id, ReceiptData& receipt)            
{
    if (!smsc_id || !smsc_id[0]) return false;
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
    smsc_log_info(logger, "Input queue closed");
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
    smsc_log_info(logger, "Output queue closed");
    return false;
}

}}

