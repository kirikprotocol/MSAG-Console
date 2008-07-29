#include "TaskProcessor.h"
#include <exception>
#include <list>
#include <sstream>
#include <util/timeslotcounter.hpp>
#include <util/config/region/RegionFinder.hpp>

extern bool isMSISDNAddress(const char* string);

namespace smsc { namespace infosme 
{

/* ---------------------------- TaskProcessor ---------------------------- */

TaskProcessor::TaskProcessor(ConfigView* config)
    : TaskProcessorAdapter(), InfoSmeAdmin(), Thread(),
      logger(Logger::getInstance("smsc.infosme.TaskProcessor")), 
      bStarted(false), bNeedExit(false),
      messageSender(0), 
      responceWaitTime(0), receiptWaitTime(0), dsStatConnection(0),
      statistics(0), protocolId(0), svcType(0), address(0),
      unrespondedMessagesMax(1), unrespondedMessagesSleep(10)
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
    switchTimeout = config->getInt("tasksSwitchTimeout");
    if (switchTimeout <= 0) 
        throw ConfigException("Task switch timeout should be positive");

    try { unrespondedMessagesMax = config->getInt("unrespondedMessagesMax"); } catch (...) {};
    if (unrespondedMessagesMax <= 0) {
      unrespondedMessagesMax = 1;
      smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value is invalid. Using default %d",
                    unrespondedMessagesMax);
    }
    if (unrespondedMessagesMax > 500) {
      smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value '%d' is too big. "
                    "The preffered max value is 500", unrespondedMessagesMax);
    }
    
    try { unrespondedMessagesSleep = config->getInt("unrespondedMessagesSleep"); } catch (...) {};
    if (unrespondedMessagesSleep <= 0) {
      unrespondedMessagesSleep = 10;
      smsc_log_warn(logger, "'unrespondedMessagesSleep' value is invalid. Using default %dms",
                    unrespondedMessagesSleep);
    }
    if (unrespondedMessagesSleep > 500) {
      smsc_log_warn(logger, "Parameter 'unrespondedMessagesSleep' value '%d' is too big. "
                    "The preffered max value is 500ms", unrespondedMessagesSleep);
    }

    std::auto_ptr<ConfigView> tasksThreadPoolCfgGuard(config->getSubConfig("TasksThreadPool"));
    taskManager.init(tasksThreadPoolCfgGuard.get());   // loads up thread pool for tasks
    std::auto_ptr<ConfigView> eventsThreadPoolCfgGuard(config->getSubConfig("EventsThreadPool"));
    eventManager.init(eventsThreadPoolCfgGuard.get()); // loads up thread pool for events
    
    std::auto_ptr<ConfigView> providerCfgGuard(config->getSubConfig("DataProvider"));
    provider.init(providerCfgGuard.get());
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("systemDataSource"));

    smsc_log_info(logger, "Loading tasks ...");
    std::auto_ptr<ConfigView> tasksCfgGuard(config->getSubConfig("Tasks"));
    ConfigView* tasksCfg = tasksCfgGuard.get();
    std::auto_ptr< std::set<std::string> > setGuard(tasksCfg->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();

    storeLocation=config->getString("storeLocation");
    if(storeLocation.length())
    {
      if(*storeLocation.rbegin()!='/')
      {
        storeLocation+='/';
      }
    }

    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* taskId = (const char *)i->c_str();
            if (!taskId || taskId[0] == '\0')
                throw ConfigException("Task id empty or wasn't specified");
            smsc_log_info(logger, "Loading task '%s' ...", taskId);
            
            std::auto_ptr<ConfigView> taskConfigGuard(tasksCfg->getSubConfig(taskId));
            ConfigView* taskConfig = taskConfigGuard.get();

            bool delivery = false;
            try { delivery = taskConfig->getBool("delivery"); }
            catch (ConfigException& ce) { delivery = false; }

            std::string location=storeLocation+taskId;
            if(!buf::File::Exists(location.c_str()))
            {
              buf::File::MkDir(location.c_str());
            }
            
            DataSource* taskDs = 0;
            if (!delivery)
            {
                const char* dsId = taskConfig->getString("dsId");
                if (!dsId || dsId[0] == '\0')
                    throw ConfigException("DataSource id for task '%s' empty or wasn't specified",
                                          taskId);
                taskDs = provider.getDataSource(dsId);
                if (!taskDs)
                    throw ConfigException("Failed to obtail DataSource driver '%s' for task '%s'", 
                                          dsId, taskId);
            }
            uint32_t taskIdVal=atoi(taskId);
            Task* task = new Task(taskConfig, taskIdVal, location,taskDs );
            if (task && !putTask(task)) {
                task->finalize();
                throw ConfigException("Failed to add task. Task with id '%s' already registered.",
                                      taskId);
            }
        }
        catch (ConfigException& exc)
        {
            smsc_log_error(logger, "Load of tasks failed ! Config exception: %s", exc.what());
            throw;
        }
    }
    smsc_log_info(logger, "Tasks loaded.");

    smsc_log_info(logger, "Loading task schedules ...");
    std::auto_ptr<ConfigView> schedulerCfgGuard(config->getSubConfig("Schedules"));
    ConfigView* schedulerCfg = schedulerCfgGuard.get();
    scheduler.init(this, schedulerCfg);
    smsc_log_info(logger, "Task schedules loaded.");
    
    smsc_log_info(logger, "Load success.");

    jstore.Init((storeLocation+"mapping.bin").c_str(),config->getInt("mappingRollTime"),config->getInt("mappingMaxChanges"));
    
    statistics = new StatisticsManager(config->getString("statStoreLocation"),this);
    if (statistics) statistics->Start();
    scheduler.Start();
}
TaskProcessor::~TaskProcessor()
{
  jstore.Stop();
  scheduler.Stop();
  this->Stop();
  taskManager.Stop();
  eventManager.Stop();
  
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

bool TaskProcessor::putTask(Task* task)
{
    __require__(task);
    MutexGuard guard(tasksLock);

    if (tasks.Exist(task->getId())) return false;
    tasks.Insert(task->getId(), task);
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
TaskGuard TaskProcessor::getTask(uint32_t taskId)
{
    MutexGuard guard(tasksLock);

    if (!tasks.Exist(taskId)) return TaskGuard(0);
    Task* task = tasks.Get(taskId);
    return TaskGuard((task && !task->isFinalizing()) ? task:0);
}

void TaskProcessor::resetWaitingTasks()
{
    MutexGuard guard(tasksLock);
    
    int key = 0; 
    Task* task = 0;
    IntHash<Task*>::Iterator it=tasks.First();
    while (it.Next(key, task))
        if (task) task->resetWaiting();
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
          MutexGuard snGuard(taskIdsBySeqNumMonitor);
          taskIdsBySeqNum.Empty();
          taskIdsBySeqNumMonitor.notifyAll();
        }
        {
          MutexGuard respGuard(responceWaitQueueLock);
          responceWaitQueue.Clean();
        }
        resetWaitingTasks();
        bNeedExit = false;
        awake.Wait(0);
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
        awake.Signal();
        exited.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
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
            int key = 0;
            Task* task = 0;
            IntHash<Task*>::Iterator it=tasks.First();
            while (!bNeedExit && it.Next(key, task))
                if (task && task->isReady(currentTime, true)) {
                    taskGuards.Push(new TaskGuard(task));
                    task->currentPriorityFrameCounter = 0;
                    task->resetSuspendedRegions();
                }
        }

        int processed = 0;
        while (taskGuards.Count()>0 && !bNeedExit)
        {
            TaskGuard* taskGuard = 0;
            taskGuards.Shift(taskGuard);
            if (!taskGuard) continue;

            if (!bNeedExit)
            {
                Task* task = taskGuard->get();
                if (task && !task->isFinalizing() && task->isEnabled() &&
                    task->currentPriorityFrameCounter < task->getPriority())
                {
                    task->currentPriorityFrameCounter++;
                    smsc_log_debug(logger, "TaskProcessor::Execute::: processTask for taskId=%d", task->getId());
                    if (!processTask(task)) {
                        task->currentPriorityFrameCounter = task->getPriority();
                        if (!task->isEnabled()) task->setEnabled(false); // to reset inProcess
                    }
                    else processed++;
                }
            }
            delete taskGuard;
        }

        if (bNeedExit) break;

        processWaitingEvents(time(NULL)); // ?? or time(NULL)
        if (!bNeedExit && processed <= 0) awake.Wait(switchTimeout);
    }
    exited.Signal();
    return 0;
}

TaskProcessor::traffic_control_res_t
TaskProcessor::controlTrafficSpeedByRegion(Task* task, Message& message)
{
  uint32_t taskId = task->getId();

  smsc_log_debug(logger, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: check region(regionId=%s) bandwidth limit exceeding", taskId, message.regionId.c_str());
  const smsc::util::config::region::Region* region = smsc::util::config::region::RegionFinder::getInstance().getRegionById(message.regionId);

  timeSlotsHashByRegion_t::iterator iter = _timeSlotsHashByRegion.find(message.regionId);
  if ( iter == _timeSlotsHashByRegion.end() ) {
    smsc_log_debug(logger, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: insert timeSlot to hash for regionId=%s", taskId, message.regionId.c_str());
    std::pair<timeSlotsHashByRegion_t::iterator, bool> insRes = _timeSlotsHashByRegion.insert(std::make_pair(message.regionId, new TimeSlotCounter<int>(1,1)));
    iter = insRes.first;
  }
  TimeSlotCounter<int>* outgoing = iter->second;
  int out = outgoing->Get();

  bool regionTrafficLimitReached = (out >= region->getBandwidth());
  smsc_log_debug(logger, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: regionTrafficLimitReached=%d, region bandwidth=%d, current sent messages during one second=%d", taskId, regionTrafficLimitReached, region->getBandwidth(), out);
  // check max messages per sec. limit. if limit was reached then put message to queue of suspended messages.
  if ( regionTrafficLimitReached ) {
    task->putToSuspendedMessagesQueue(message);
    return TRAFFIC_SUSPENDED;
  } else {
    outgoing->Inc();
    return TRAFFIC_CONTINUED;
  }
}

struct MessageGuard{
  MessageGuard(Task* argTask,uint64_t argMsgId):task(argTask),msgId(argMsgId),messageProcessed(false)
  {
  }

  ~MessageGuard()
  {
    if(!messageProcessed)
    {
      time_t retryTime=task->getInfo().retryTime;
      if(retryTime==0)
      {
        retryTime=60*60;
      }
      task->retryMessage(msgId,time(NULL)+retryTime);
    }
  }

  void processed()
  {
    messageProcessed=true;
  }

  Task* task;
  uint64_t msgId;
  bool messageProcessed;
};

bool TaskProcessor::processTask(Task* task)
{
    __require__(task);

    TaskInfo info = task->getInfo();
    Message message;
    if (!task->getNextMessage(message))
    {
        //smsc_log_debug(logger, "No messages found for task '%s'", info.id.c_str());
        return false;
    }

    MessageGuard msguard(task,message.id);

    MutexGuard msGuard(messageSenderLock);
    if (messageSender)
    {
        if ( controlTrafficSpeedByRegion(task, message) != TRAFFIC_SUSPENDED ) {
          int seqNum = messageSender->getSequenceNumber();
          smsc_log_debug(logger, "TaskId=[%d/%s]: Sending message #%llx,sq=%d for '%s': %s", 
                         info.uid,info.name.c_str(), message.id, seqNum, message.abonent.c_str(), message.message.c_str());
          {
            {
              MutexGuard snGuard(taskIdsBySeqNumMonitor);
              int seqNumsCount;
              while ((seqNumsCount=taskIdsBySeqNum.Count()) > unrespondedMessagesMax && !bNeedExit) {
                int difference = seqNumsCount - unrespondedMessagesMax;
                taskIdsBySeqNumMonitor.wait(difference*unrespondedMessagesSleep);
              }
              if (bNeedExit) return false;

              if (taskIdsBySeqNum.Exist(seqNum))
              {
                smsc_log_warn(logger, "Sequence id=%d was already used !", seqNum);
                taskIdsBySeqNum.Delete(seqNum);
              }
              taskIdsBySeqNum.Insert(seqNum, TaskMsgId(info.uid, message.id));
            }
            MutexGuard respGuard(responceWaitQueueLock);
            responceWaitQueue.Push(ResponceTimer(time(NULL)+responceWaitTime, seqNum));
          }
        
          if (!messageSender->send(message.abonent, message.message, info, seqNum))
          {
            smsc_log_error(logger, "Failed to send message #%llx for '%s'", 
                           message.id, message.abonent.c_str());

            MutexGuard snGuard(taskIdsBySeqNumMonitor);
            if (taskIdsBySeqNum.Exist(seqNum))
            {
              taskIdsBySeqNum.Delete(seqNum);
              taskIdsBySeqNumMonitor.notifyAll();
            }
            return false;
          }
          msguard.processed();
          smsc_log_info(logger, "TaskId=[%d/%s]: Sent message #%llx sq=%d for '%s'", 
                        info.uid,info.name.c_str(), message.id, seqNum, message.abonent.c_str());
        } else {
          const smsc::util::config::region::Region* region = smsc::util::config::region::RegionFinder::getInstance().getRegionById(message.regionId);
          smsc_log_info(logger, "TaskId=[%d/%s]: Traffic for region %s with id %s was suspended",
                        info.uid,info.name.c_str(), region->getName().c_str(), region->getId().c_str());
          return false;
        }
    }
    else
    {
        smsc_log_error(logger, "No messageSender defined !!!");
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
            if (responceWaitQueue.Count() > 0)
            {
                timer = responceWaitQueue[0];
                if (timer.timer > time)
                {
                  break;
                }
                responceWaitQueue.Shift(timer);
            }
            else
            {
              break;
            }
        }

        bool needProcess = false;
        {
          MutexGuard guard(taskIdsBySeqNumMonitor);
          needProcess = taskIdsBySeqNum.Exist(timer.seqNum);
        }
        if (needProcess)
        {
          processResponce(timer.seqNum, false, true, true, "", true);
        }
        
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
            if (receiptPtr) { 
              smsc_log_warn(logger, "%s for smscId=%s wasn't received and timed out!", 
                            ((receiptPtr->receipted) ? "Receipt":"Responce"),
                            timer.smscId.c_str());
              needProcess = true;
            }
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

void TaskProcessor::processMessage(Task* task, uint64_t msgId,
                                   bool delivered, bool retry, bool immediate)
{
    __require__(task);

    if (delivered)
    {
        task->finalizeMessage(msgId, DELIVERED);
        statistics->incDelivered(task->getInfo().uid);
    }
    else
    {
        TaskInfo info = task->getInfo();
        if (retry && (immediate || (info.retryOnFail && info.retryTime > 0)))
        {
            time_t nextTime = time(NULL)+((immediate) ? 0:info.retryTime);

            if ((info.endDate>0 && nextTime >=info.endDate) ||
                (info.validityDate>0 && nextTime>=info.validityDate))
            {
                task->finalizeMessage(msgId, EXPIRED);
                statistics->incFailed(info.uid);
            } 
            else
            {
                if (!task->retryMessage(msgId, nextTime)) {
                    smsc_log_warn(logger, "Message #%lld not found for retry.", msgId);
                    statistics->incFailed(info.uid);
                } 
                else if (!immediate) statistics->incRetried(info.uid);
            }
        }
        else
        { 
            task->finalizeMessage(msgId, FAILED);
            statistics->incFailed(info.uid);
        }
    }
}

const char* CREATE_ID_MAPPING_STATEMENT_ID = "CREATE_ID_MAPPING_STATEMENT_ID";
const char* CREATE_ID_MAPPING_STATEMENT_SQL = (const char*)
"INSERT INTO INFOSME_ID_MAPPING (ID, SMSC_ID, TASK_ID) VALUES (:ID, :SMSC_ID, :TASK_ID)";

const char* GET_ID_MAPPING_STATEMENT_ID = "GET_ID_MAPPING_STATEMENT_ID";
const char* GET_ID_MAPPING_STATEMENT_SQL = (const char*)
"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING WHERE SMSC_ID=:SMSC_ID";

const char* DEL_ID_MAPPING_STATEMENT_ID = "DEL_ID_MAPPING_STATEMENT_ID";
const char* DEL_ID_MAPPING_STATEMENT_SQL = (const char*)
"DELETE FROM INFOSME_ID_MAPPING WHERE ID=:ID";

void TaskProcessor::processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                    std::string smscId, bool internal)
{
    if (!internal) smsc_log_info(logger, "Responce: seqNum=%d, accepted=%d, retry=%d, immediate=%d",
                                 seqNum, accepted, retry, immediate);
    else smsc_log_info(logger, "Responce for seqNum=%d is timed out.", seqNum);

    TaskMsgId tmIds;
    {   
        TaskMsgId* tmIdsPtr = 0;
        MutexGuard snGuard(taskIdsBySeqNumMonitor);
        if (!(tmIdsPtr = taskIdsBySeqNum.GetPtr(seqNum))) {
            if (!internal) smsc_log_warn(logger, "processResponce(): Sequence number=%d is unknown !", seqNum);
            return;
        }
        tmIds = *tmIdsPtr;
        taskIdsBySeqNum.Delete(seqNum);
        taskIdsBySeqNumMonitor.notifyAll();
    }
    
    TaskGuard taskGuard = getTask(tmIds.getTaskId()); 
    Task* task = taskGuard.get();
    if (!task) {
        if (!internal) smsc_log_warn(logger, "Unable to locate task '%d' for sequence number=%d", 
                                     tmIds.taskId, seqNum);
        return;
    }
    TaskInfo info = task->getInfo();

    if (!accepted || internal)
    {
        if (retry && (immediate || (info.retryOnFail && info.retryTime > 0)))
        {
            time_t nextTime = time(NULL)+((immediate) ? 0:info.retryTime);

            if ((info.endDate>0 && nextTime >=info.endDate) ||
                (info.validityDate>0 && nextTime>=info.validityDate))
            {
                task->finalizeMessage(tmIds.msgId, EXPIRED);
                statistics->incFailed(info.uid);
            } 
            else
            {
                if (!task->retryMessage(tmIds.msgId, nextTime)) {
                    smsc_log_warn(logger, "Message #%lld not found for retry.", tmIds.msgId);
                    statistics->incFailed(info.uid);
                } 
                else if (!immediate) statistics->incRetried(info.uid);
            }
        }
        else
        {
            task->finalizeMessage(tmIds.msgId, FAILED);
            statistics->incFailed(info.uid);
        }
    }
    else
    {
        if (info.transactionMode) {
            task->finalizeMessage(tmIds.msgId, DELIVERED);
            statistics->incDelivered(info.uid);
            return;
        }
        
        const char* smsc_id = smscId.c_str();
        
        try
        {
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
                if (!task->enrouteMessage(tmIds.msgId))
                    throw Exception("Message #%lld not found (doEnroute).", tmIds.msgId);

                TaskIdMsgId timi;
                timi.msgId=tmIds.msgId;
                timi.taskId=info.uid;
                jstore.Insert(atol(smsc_id),timi);
                idMappingCreated = true;
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
                smsc_log_debug(logger, "Receipt come when responce is in process");
                if (idMappingCreated)
                {
                  jstore.Delete(atol(smsc_id));
                }
                
                processMessage(task, tmIds.msgId, receipt.delivered, receipt.retry);
            }
        }
        catch (std::exception& exc) {
            smsc_log_error(logger, "Failed to process responce. Details: %s", exc.what());
        }
        catch (...) {
            smsc_log_error(logger, "Failed to process responce.");
        }
    }
}

void TaskProcessor::processReceipt (std::string smscId, bool delivered, bool retry, bool internal)
{
    const char* smsc_id = smscId.c_str();

    if (!internal) smsc_log_info(logger, "Receipt : smscId=%s, delivered=%d, retry=%d",
                                 smsc_id, delivered, retry);
    else smsc_log_info(logger, "Responce/Receipt for smscId=%s is timed out. Cleanup.",smsc_id);
    
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
    
    try
    {

      
      /*std::auto_ptr<Statement> getMapping(connection->createStatement(GET_ID_MAPPING_STATEMENT_SQL));
        if (!getMapping.get())
            throw Exception("processReceipt(): Failed to create statement for ids mapping.");
    
        getMapping->setString(1, smsc_id);
        std::auto_ptr<ResultSet> rsGuard(getMapping->executeQuery());
        ResultSet* rs = rsGuard.get();*/
        
          //while(rs->fetchNext())

      
      TaskIdMsgId timi;
      uint64_t msgId;
      uint32_t taskId;
      bool needProcess = false;
      if(jstore.Lookup(atol(smsc_id),timi))
      {
        msgId = timi.msgId;
        taskId = timi.taskId;
        MutexGuard guard(receiptsLock);
        ReceiptData* receiptPtr = receipts.GetPtr(smsc_id);
        if (receiptPtr)
        { 
          receipts.Delete(smsc_id);
          needProcess = true;
        }
      }
      
      if (needProcess)
      {
      
        jstore.Delete(atol(smsc_id));
      
        TaskGuard taskGuard = getTask(taskId); 
        Task* task = taskGuard.get();
        if (!task)
          throw Exception("processReceipt(): Unable to locate task '%d' for smscId=%s",
                          taskId, smsc_id);

        processMessage(task, msgId, delivered, retry, internal);
      }
      
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Failed to process receipt. Details: %s", exc.what());
    }
    catch (...)
    {
        smsc_log_error(logger, "Failed to process receipt.");
    }
    
}

/* ------------------------ Admin interface implementation ------------------------ */ 

void TaskProcessor::addTask(uint32_t taskId)
{

    Task* task = 0; bool delivery = false;
    try
    {
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
          smsc_log_info(logger,"creating new dir:'%s' for taskId=%u",location.c_str(),taskId);
          buf::File::MkDir(location.c_str());
        }

        task = new Task(&taskConfig, taskId, location, taskDs);
        if (!task) 
            throw Exception("New task create failed");
        if (!addTask(task))
            throw ConfigException("Failed to add task. Task with id '%u' already registered.",
                                  taskId);
       
    } catch (std::exception& exc) {
        if (task && !delivery) task->destroy();
        smsc_log_error(logger, "Failed to add task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) {
        if (task && !delivery) task->destroy();
        smsc_log_error(logger, "Failed to add task '%d'. Cause is unknown", taskId);
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
        smsc_log_error(logger, "Failed to remove task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) {
        smsc_log_error(logger, "Failed to remove task '%d'. Cause is unknown", taskId);
        throw Exception("Cause is unknown");
    }
}
void TaskProcessor::changeTask(uint32_t taskId)
{
    try
    {
        Manager::reinit();
        Manager& config = Manager::getInstance();
        char taskSection[1024];
        sprintf(taskSection, "InfoSme.Tasks.%d", taskId);
        ConfigView taskConfig(config, taskSection);
        TaskGuard tg(getTask(taskId));
        Task* task = tg.get();
        task->update(&taskConfig);

        /*
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
        
        if (!remTask(taskId))
        {
          smsc_log_warn(logger, "Failed to change task. Task with id '%d' wasn't registered.", taskId);
        }
        std::string location=storeLocation;
        char buf[32];
        sprintf(buf,"%u/",taskId);
        location+=buf;
        if(!buf::File::Exists(location.c_str()))
        {
          smsc_log_info(logger,"creating new dir:'%s' for taskId=%u",location.c_str(),taskId);
          buf::File::MkDir(location.c_str());
        }
        task = new Task(&taskConfig, taskId, location, taskDs);
        if (!task) 
        {
          throw Exception("New task create failed");
        }
        if (!putTask(task))
        {
          smsc_log_warn(logger, "Failed to change task with id '%d'. Task was re-registered", taskId);
          if (!delivery) task->destroy();
        }
        */
    } catch (std::exception& exc)
    {
        //if (task && !delivery) task->destroy();
        smsc_log_error(logger, "Failed to change task '%d'. Details: %s", taskId, exc.what());
        throw;
    } catch (...) 
    {
        //if (task && !delivery) task->destroy();
        smsc_log_error(logger, "Failed to change task '%d'. Cause is unknown", taskId);
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
        smsc_log_error(logger, "Failed to add schedule '%s'. Details: %s", schedule_id, exc.what());
        throw;
    } catch (std::exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(logger, "Failed to add schedule '%s'. Details: %s", schedule_id, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        if (schedule) delete schedule;
        smsc_log_error(logger, "Failed add schedule '%s'. Cause is unknown", schedule_id);
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
            smsc_log_warn(logger, "Failed to change schedule with id '%s'. Schedule was re-registered", schedule_id);
        }
    
    } catch (Exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(logger, "Failed to change schedule '%s'. Details: %s", schedule_id, exc.what());
        throw;
    } catch (std::exception& exc) {
        if (schedule) delete schedule;
        smsc_log_error(logger, "Failed to change schedule '%s'. Details: %s", schedule_id, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        if (schedule) delete schedule;
        smsc_log_error(logger, "Failed change schedule '%s'. Cause is unknown", schedule_id);
        throw Exception("Cause is unknown");
    }
}

void TaskProcessor::addDeliveryMessages(uint32_t taskId,
                                        uint8_t msgState,
                                        const std::string& abonentAddress,
                                        time_t messageDate,
                                        const std::string& msg)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::addDeliveryMessages::: can't get task by taskId='%d'", taskId);
  task->insertDeliveryMessage(msgState, abonentAddress, messageDate, msg);
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
    smsc_log_debug(logger,"msgId=#%s changed to #%llx",recordId.c_str(),newMsgId);
  }else
  {
    smsc_log_debug(logger,"changeDeliveryMessageInfoByRecordId failed msgId=#%s",recordId.c_str());
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


/*
void TaskProcessor::insertRecordIntoTasksStat(uint32_t taskId,
                                              uint32_t period,
                                              uint32_t generated,
                                              uint32_t delivered,
                                              uint32_t retried,
                                              uint32_t failed)
{
  std::auto_ptr<Statement> statement(dsIntConnection->createStatement(INSERT_TASK_STAT_STATE_SQL));
  if (!statement.get())
    throw Exception("Failed to obtain statement for statistics update");

  char buf[32];
  sprintf(buf,"%u",taskId);

  statement->setString(1, buf);
  statement->setUint32(2, period);
  statement->setUint32(3, generated);
  statement->setUint32(4, delivered);
  statement->setUint32(5, retried);
  statement->setUint32(6, failed);

  statement->executeUpdate();
  !!TODO!! STATS! :(
  
}
  */

Array<std::string> TaskProcessor::getTaskMessages(const uint32_t taskId,
                                                  const InfoSme_T_SearchCriterion& searchCrit)
{
  try {
    TaskGuard taskGuard = getTask(taskId);
    Task* task = taskGuard.get();
    if (!task) throw Exception("TaskProcessor::getTaskMessages::: can't get task by taskId='%d'", taskId);
    return task->selectDeliveryMessagesByCompositCriterion(searchCrit);
  } catch (std::exception& ex) {
    smsc_log_error(logger, "TaskProcessor::getTaskMessages::: catch exception=[%s]", ex.what());
    throw;
  } catch (...) {
    smsc_log_error(logger, "TaskProcessor::getTaskMessages::: catch unexpected exception");
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

/*
Array<std::string> TaskProcessor::getTasksStatistic(const InfoSme_Tasks_Stat_SearchCriterion& searchCrit)
{
  std::auto_ptr<Statement> selectMessage(dsIntConnection->createStatement(DO_FULL_TABLESCAN_TASKS_STAT_STATEMENT_SQL));
  if (!selectMessage.get())
    throw Exception("getTasksStatistic(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());

  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");

  typedef std::list<TaskStatDescription> TasksStatList_t;
  TasksStatList_t statisticsList;

  int fetched = 0;
  while (rs->fetchNext()) {
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      smsc_log_debug(logger, "TaskProcessor::getTasksStatistic::: add statistic [%s,%d,%d,%d,%d,%d] into statisticsList", rs->getString(1), rs->getUint32(2), rs->getUint32(3), rs->getUint32(4), rs->getUint32(5), rs->getUint32(6));
      statisticsList.push_back(TaskStatDescription(rs->getString(1),
                                                   rs->getUint32(2),
                                                   rs->getUint32(3),
                                                   rs->getUint32(4),
                                                   rs->getUint32(5),
                                                   rs->getUint32(6)));
    }
  }

  statisticsList.sort(orderBinaryPredicate);

  TasksStatList_t accumulatedStatisticsList;

  TasksStatList_t::iterator iter = statisticsList.begin();

  if ( iter != statisticsList.end() ) {
    TaskStatDescription prevElement(*iter);
    TaskStatDescription taskStatResult(*iter);
    while ( ++iter != statisticsList.end() ) {
      if ( iter->period != prevElement.period ) {
        accumulatedStatisticsList.push_back(taskStatResult);
        taskStatResult = *iter;
      } else
        taskStatResult = taskStatResult + *iter;
      prevElement = *iter;
    }
    accumulatedStatisticsList.push_back(taskStatResult);
  }

  Array<std::string> tasksStat;
  for(TasksStatList_t::iterator iter=accumulatedStatisticsList.begin();
      iter!=accumulatedStatisticsList.end(); ++iter) {
    std::ostringstream statisticBuf;
    statisticBuf << iter->period << "|"
                 << iter->generated << "|"
                 << iter->delivered << "|"
                 << iter->retried << "|"
                 << iter->failed;
    tasksStat.Push(statisticBuf.str());
  }

  return tasksStat;
}*/

void
TaskProcessor::endDeliveryMessagesGeneration(uint32_t taskId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  if (!task) throw Exception("TaskProcessor::endDeliveryMessagesGeneration::: can't get task by taskId='%d'", taskId);
  task->endDeliveryMessagesGeneration();
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
}

TaskInfo TaskProcessor::getTaskInfo(uint32_t taskId)
{
  TaskGuard taskGuard = getTask(taskId);
  Task* task = taskGuard.get();
  return task?task->getInfo():TaskInfo();
}


}}

