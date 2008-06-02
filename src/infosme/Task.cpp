#include "Task.h"
#include "SQLAdapters.h"
#include "TaskLock.hpp"
#include <time.h>
#include <sstream>
#include <list>
#include <util/sleep.h>
#include <sms/sms.h>
#include <util/config/region/Region.hpp>
#include <util/config/region/RegionFinder.hpp>

extern bool isMSISDNAddress(const char* string);

namespace smsc { namespace infosme 
{

const char* USER_QUERY_STATEMENT_ID           = "%s_USER_QUERY_STATEMENT_ID";   // Own

const char* DELETE_GENERATING_STATEMENT_ID    = "DELETE_GENERATING_STATEMENT_ID";
const char* INSERT_GENERATING_STATEMENT_ID    = "INSERT_GENERATING_STATEMENT_ID"; 
const char* DELETE_NEW_MESSAGES_STATEMENT_ID  = "%s_DELETE_NEW_MESSAGES_STATEMENT_ID";
const char* SELECT_MESSAGES_STATEMENT_ID      = "%s_SELECT_MESSAGES_STATEMENT_ID";
const char* NEW_MESSAGE_STATEMENT_ID          = "%s_NEW_MESSAGE_STATEMENT_ID";
const char* CLEAR_MESSAGES_STATEMENT_ID       = "%s_CLEAR_MESSAGES_STATEMENT_ID";
const char* RESET_MESSAGES_STATEMENT_ID       = "%s_RESET_MESSAGES_STATEMENT_ID";
const char* DO_STATE_MESSAGE_STATEMENT_ID     = "%s_DO_STATE_MESSAGE_STATEMENT_ID";
const char* DO_RETRY_MESSAGE_STATEMENT_ID     = "%s_DO_RETRY_MESSAGE_STATEMENT_ID";
const char* DO_DELETE_MESSAGE_STATEMENT_ID    = "%s_DO_DELETE_MESSAGE_STATEMENT_ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_ID   = "%s_DO_ENROUTE_MESSAGE_STATEMENT_ID";

const char* DELETE_GENERATING_STATEMENT_SQL   = "DELETE FROM INFOSME_GENERATING_TASKS WHERE TASK_ID=:TASK_ID";
const char* INSERT_GENERATING_STATEMENT_SQL   = "INSERT INTO INFOSME_GENERATING_TASKS TASK_ID VALUES(:TASK_ID)";
const char* DELETE_NEW_MESSAGES_STATEMENT_SQL = "DELETE FROM %s WHERE STATE=:NEW";
const char* CLEAR_MESSAGES_STATEMENT_SQL      = "DELETE FROM %s WHERE STATE=:NEW AND ABONENT=:ABONENT";
const char* NEW_SD_INDEX_STATEMENT_SQL        = "CREATE INDEX %s_SD_IDX ON %s (STATE, SEND_DATE)";
const char* NEW_AB_INDEX_STATEMENT_SQL        = "CREATE INDEX %s_AB_IDX ON %s (STATE, ABONENT)";
const char* RESET_MESSAGES_STATEMENT_SQL      = "UPDATE %s SET STATE=:NEW WHERE STATE=:WAIT";
const char* DO_STATE_MESSAGE_STATEMENT_SQL    = "UPDATE %s SET STATE=:STATE WHERE ID=:ID";
const char* DO_RETRY_MESSAGE_STATEMENT_SQL    = "UPDATE %s SET STATE=:NEW, SEND_DATE=:SEND_DATE WHERE ID=:ID";
const char* DO_DELETE_MESSAGE_STATEMENT_SQL   = "DELETE FROM %s WHERE ID=:ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_SQL  = "UPDATE %s SET STATE=:ENROUTE WHERE ID=:ID AND STATE=:WAIT";
const char* DROP_TABLE_STATEMENT_SQL          = "DROP TABLE %s";
const char* NEW_TABLE_STATEMENT_SQL           = "CREATE TABLE %s (\n"
                                                "ID             NUMBER          NOT NULL,\n"
                                                "STATE          NUMBER(3)       NOT NULL,\n"
                                                "ABONENT        VARCHAR2(30)    NOT NULL,\n"
                                                "SEND_DATE      DATE            NOT NULL,\n"
                                                "MESSAGE        VARCHAR2(2000)  NULL,    \n"
                                                "PRIMARY KEY    (ID)                     \n"
                                                ")";
const char* NEW_MESSAGE_STATEMENT_SQL         = "INSERT INTO %s (ID, STATE, ABONENT, SEND_DATE, MESSAGE) "
                                                "VALUES (INFOSME_MESSAGES_SEQ.NEXTVAL, :STATE, :ABONENT, "
                                                ":SEND_DATE, :MESSAGE)";
const char* SELECT_MESSAGES_STATEMENT_SQL     = "SELECT ID, ABONENT, MESSAGE FROM %s WHERE "
                                                "STATE=:STATE AND SEND_DATE<=:SEND_DATE ORDER BY SEND_DATE ASC";

const char* DROP_INFOSME_T_INDEX_STATEMENT_SQL = "DROP INDEX ON %s";

Task::Task(ConfigView* config, uint32_t taskId, std::string location, 
           DataSource* _dsOwn)
    : logger(Logger::getInstance("smsc.infosme.Task")), formatter(0),
      usersCount(0), bFinalizing(false), bSelectedAll(false), dsOwn(_dsOwn),store(location),
      bInProcess(false), bInGeneration(false), bGenerationSuccess(false),
      infoSme_T_storageWasDestroyed(false), lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
   
{
  store.Init();
  init(config, taskId);
  formatter = new OutputFormatter(info.msgTemplate.c_str());
  trackIntegrity(true, true); // delete flag & generated messages
}
Task::~Task()
{
    if (formatter) delete formatter;

    if (dsOwn)
    {
      char buf[128];
      char idBuf[32];
      sprintf(idBuf,"%d",info.uid);
      sprintf(buf,USER_QUERY_STATEMENT_ID,idBuf);
      dsOwn->closeRegisteredQueries(buf);
    }
}

void Task::init(ConfigView* config, uint32_t taskId)
{
    __require__(config);

    const int MAX_PRIORITY_VALUE = 1000;

    info.uid = taskId;
    try { info.name = config->getString("name"); } catch (...) {}
    info.enabled = config->getBool("enabled");
    info.priority = config->getInt("priority");
    if (info.priority <= 0 || info.priority > MAX_PRIORITY_VALUE)
        throw ConfigException("Task priority should be positive and less than %d.", 
                              MAX_PRIORITY_VALUE);
    try {
      info.address = config->getString("address"); 
    }
    catch (...)
    { 
        smsc_log_warn(logger, "<address> parameter missed for task '%d'. "
                              "Using global definitions", info.uid);
        info.address = "";
    }
    info.retryOnFail = config->getBool("retryOnFail");
    info.replaceIfPresent = config->getBool("replaceMessage");
    info.transactionMode = config->getBool("transactionMode");
    info.trackIntegrity = config->getBool("trackIntegrity");
    info.keepHistory = config->getBool("keepHistory");
    info.endDate = parseDateTime(config->getString("endDate"));

    /*
    !!TODO!!
    if (info.endDate>0 && time(0)>=info.endDate)
    {
      // preload InfoSme_T_ storage without index building
      Connection* connection = 0;
      connection = dsInt->getConnection();

      std::string createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
      std::auto_ptr<Statement> statementGuard(connection->createStatement(createTableSql.c_str()));
      Statement* statement = statementGuard.get();
      if (!statement) 
        throw Exception("Failed to create table statement.");
      statement->execute();  

      // and set flag in order to don't try destroy storage needed only for queries execution
      infoSme_T_storageWasDestroyed = true;
    }
    */

    info.retryTime = parseTime(config->getString("retryTime"));
    if (info.retryOnFail && info.retryTime <= 0)
        throw ConfigException("Task retry time specified incorrectly."); 
    info.validityPeriod = parseTime(config->getString("validityPeriod"));
    info.validityDate = parseDateTime(config->getString("validityDate"));
    if (info.validityPeriod <= 0 && info.validityDate <= 0)
        throw ConfigException("Message validity period/date specified incorrectly.");
    info.activePeriodStart = parseTime(config->getString("activePeriodStart"));
    info.activePeriodEnd = parseTime(config->getString("activePeriodEnd"));
    if ((info.activePeriodStart < 0 && info.activePeriodEnd >= 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd < 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd >= 0 && 
         info.activePeriodStart >= info.activePeriodEnd))
        throw ConfigException("Task active period specified incorrectly."); 
    
    const char* awd = 0;
    try { awd = config->getString("activeWeekDays"); } 
    catch (...) { 
        smsc_log_warn(logger, "<activeWeekDays> parameter missed for task '%d'. "
                              "Using default: Mon,Tue,Wed,Thu,Fri", info.uid);
        info.activeWeekDays.weekDays = 0x7c; awd = 0;
    }
    if (awd && awd[0]) {
        if (!info.activeWeekDays.setWeekDays(awd))
            throw ConfigException("Task active week days set listed incorrectly."); 
    }
    else info.activeWeekDays.weekDays = 0;

    if (dsOwn != 0)
    {
        const char* query_sql = config->getString("query");
        if (!query_sql || query_sql[0] == '\0')
            throw ConfigException("Sql query for task empty or wasn't specified.");
        info.querySql = query_sql;
        const char* msg_template = config->getString("template");
        if (!msg_template || msg_template[0] == '\0')
            throw ConfigException("Message template for task empty or wasn't specified.");
        info.msgTemplate = msg_template;
    }
    info.svcType = "";
    if (info.replaceIfPresent)
    {
        try         { info.svcType = config->getString("svcType"); } 
        catch (...) { info.svcType = "";}
    }

    info.dsTimeout = 0;
    try { info.dsTimeout = config->getInt("dsTimeout"); } catch(...) {}
    if (info.dsTimeout < 0) info.dsTimeout = 0;
    
    info.dsUncommitedInProcess = 1;
    try { info.dsUncommitedInProcess = config->getInt("uncommitedInProcess"); } catch(...) {}
    if (info.dsUncommitedInProcess < 0) info.dsUncommitedInProcess = 1;
    info.dsUncommitedInGeneration = 1;
    try { info.dsUncommitedInGeneration = config->getInt("uncommitedInGeneration"); } catch(...) {}
    if (info.dsUncommitedInGeneration < 0) info.dsUncommitedInGeneration = 1;
    info.messagesCacheSize = 100;
    try { info.messagesCacheSize = config->getInt("messagesCacheSize"); } catch(...) {}
    if (info.messagesCacheSize <= 0) info.messagesCacheSize = 100;
    info.messagesCacheSleep = 1;
    try { info.messagesCacheSleep = config->getInt("messagesCacheSleep"); } catch(...) {}
    if (info.messagesCacheSleep <= 0) info.messagesCacheSleep = 1;
    try
    { 
      bGenerationSuccess = config->getBool("messagesHaveLoaded");
      bInGeneration = false;
    } catch (...) {}
}

void Task::doFinalization()
{
  smsc_log_error(logger, "Task::doFinalization::: taskId=%d",getId());
    {
        MutexGuard guard(finalizingLock);
        bFinalizing = true;
    }
    endGeneration();

    while (true) {
        usersCountEvent.Wait(10);
        MutexGuard guard(usersCountLock);
        if (usersCount <= 0) break;
    }
}
void Task::finalize()
{
    doFinalization();
    delete this;
}
bool Task::destroy()
{
  smsc_log_error(logger, "Task::destroy::: taskId=%d",getId());
    doFinalization();
    bool result = false;
    try
    {
      store.Delete(false);
      result = true;
    } catch (std::exception& exc)
    {
        smsc_log_error(logger, "Drop table failed for task '%d'. Reason: %s",
                       info.uid, exc.what());
    } catch (...) {
        smsc_log_error(logger, "Drop table failed for task '%d'. Reason is unknown",
                       info.uid);
    }
    delete this;
    return result;
}
bool Task::shutdown()
{
  smsc_log_error(logger, "Task::shutdown::: taskId=%d",getId());
    doFinalization(); bool result = false;
    try { resetWaiting(); result = true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Reset waiting failed for task '%d/%s'. Reason: %s",
                       info.uid, info.name.c_str(),exc.what());
    } catch (...) {
        smsc_log_error(logger, "Reset waiting failed for task '%d/%s'. Reason is unknown",
                       info.uid,info.name.c_str());
    }
    delete this;
    return result;
}

void Task::trackIntegrity(bool clear, bool del)
{
    smsc_log_debug(logger, "trackIntegrity method being called on task '%d/%s'",
                   info.uid,info.name.c_str());
    
    if (!info.trackIntegrity) return;

    try
    {
      if (clear)
      {
        if (TaskLock::getInstance().Unlock(info.uid) && del)
        {
          store.Delete(true);
        }
      }
      else
      {
        TaskLock::getInstance().Lock(info.uid);
      }
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. Failed to track task integrity. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
}

bool Task::beginGeneration(Statistics* statistics)
{
    smsc_log_debug(logger, "beginGeneration method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

    uint64_t totalGenerated = 0;

    {
        MutexGuard guard(inGenerationLock);
        if (bInGeneration || (info.trackIntegrity && isInProcess())) return false;
        bInGeneration = true; bGenerationSuccess = false;
    }

    Connection* ownConnection = 0;

    try
    {
        ownConnection = dsOwn->getConnection();
        if (!ownConnection)
            throw Exception("Failed to obtain connection to own data source.");

        trackIntegrity(false, false); // insert flag

        std::auto_ptr<Statement> userQuery(ownConnection->createStatement(info.querySql.c_str()));
        if (!userQuery.get())
        {
          throw Exception("Failed to create user query statement on own data source.");
        }

        std::auto_ptr<ResultSet> rsGuard(userQuery->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
        {
          throw Exception("Failed to obtain result set for message generation.");
        }

        SQLGetAdapter       getAdapter(rs);
        ContextEnvironment  context;

        int uncommited = 0;
        while (isInGeneration() && rs->fetchNext())
        {
            const char* abonentAddress = rs->getString(1);
            if (!abonentAddress || abonentAddress[0] == '\0' || !isMSISDNAddress(abonentAddress))
            {
                smsc_log_warn(logger, "Task '%d/%s'. Invalid abonent number '%s' selected.", 
                              info.uid,info.name.c_str(), abonentAddress ? abonentAddress:"-");
            }
            else
            {
              std::string message = "";
              formatter->format(message, getAdapter, context);
              if (message.length() > 0)
              {
                if (info.replaceIfPresent)
                {
                  //!!!TODO!!!
                }
                Message msg;
                msg.abonent=abonentAddress;
                msg.message=message;

                store.createMessage(time(NULL),msg);

                if (statistics) statistics->incGenerated(info.uid, 1);

                totalGenerated++;
              }
            }
        }
        
        {
            MutexGuard guard(inGenerationLock);
            if (info.trackIntegrity && !bInGeneration) bGenerationSuccess = false;
            else bGenerationSuccess = true;
        }

        trackIntegrity(true, false); // delete flag only
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. Messages generation process failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
        
        trackIntegrity(true, true); // delete flag & generated messages
        MutexGuard guard(inGenerationLock);
        bGenerationSuccess = false;
    }
    
    if (ownConnection) dsOwn->freeConnection(ownConnection);
    
    {
        MutexGuard guard(inGenerationLock);
        bInGeneration = false;
    }
    generationEndEvent.Signal();
    return (bGenerationSuccess && totalGenerated > 0);
}
void Task::endGeneration()
{
    smsc_log_debug(logger, "endGeneration method being called on task '%d/%s'",
                 info.uid,info.name.c_str());
    {
        MutexGuard guard(inGenerationLock);
        if (!bInGeneration) return;
        bInGeneration = false;
    }
    generationEndEvent.Wait();
}

void Task::dropNewMessages(Statistics* statistics)
{
  smsc_log_debug(logger, "dropAllMessages method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  endGeneration();
  
  try
  {
    uint32_t deleted = store.Delete(true);
    if (statistics) statistics->incFailed(info.uid, deleted);
  }
  catch (std::exception& exc)
  {
    smsc_log_error(logger, "Task '%d/%s'. Messages access failure. "
                 "Details: %s", info.uid,info.name.c_str(), exc.what());
  }
  catch (...)
  {
    smsc_log_error(logger, "Task '%d/%s'. Messages access failure.", 
                 info.uid,info.name.c_str());
  }
}

void Task::resetWaiting()
{
  smsc_log_debug(logger, "resetWaiting method being called on task '%d/%s'",
               info.uid,info.name.c_str());
  
  try
  {
    //!!!TODO!!!
  }
  catch (std::exception& exc)
  {
      smsc_log_error(logger, "Task '%d/%s'. Messages access failure. "
                   "Details: %s", info.uid,info.name.c_str(), exc.what());
  }
  catch (...)
  {
      smsc_log_error(logger, "Task '%d/%s'. Messages access failure.", info.uid,info.name.c_str());
  }
}

bool Task::retryMessage(uint64_t msgId, time_t nextTime)
{
    smsc_log_debug(logger, "retryMessage method being called on task '%d/%s' for id=%lld",
                 info.uid,info.name.c_str(), msgId);

    bool result = false;
    try
    {
      Message msg;
      uint8_t state;
      store.loadMessage(msgId,msg,state);
      store.setMsgState(msgId,DELETED);
      store.createMessage(nextTime,msg);
      result=true;
    }
    catch (std::exception& exc) {
        smsc_log_error(logger, "Task '%d/%s'. doRetry(): Messages access failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
    catch (...) {
        smsc_log_error(logger, "Task '%d/%s'. doRetry(): Messages access failure.", 
                     info.uid,info.name.c_str());
    }
    return result;
}

bool Task::finalizeMessage(uint64_t msgId, MessageState state)
{   
    if (state == NEW || state == WAIT || state == ENROUTE) {
        smsc_log_warn(logger, "Invalid state=%d to finalize message on task '%d/%s' for id=%lld",
                      state, info.uid,info.name.c_str(), msgId);
        return false;
    } else {
        smsc_log_debug(logger, "finalizeMessage(%d) method being called on task '%d/%s' for id=%lld",
                       state, info.uid,info.name.c_str(), msgId);
    }

    bool result = false;
    try
    {
      if (info.keepHistory)
      {
        store.finalizeMsg(msgId,time(NULL),state);
      }
      else
      {
        store.setMsgState(msgId,DELETED);
      }
      
      result = true;
    }
    catch (std::exception& exc) {
        smsc_log_error(logger, "Task '%d/%s'. finalizeMessage(): Messages access failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
    catch (...) {
        smsc_log_error(logger, "Task '%d/%s'. finalizeMessage(): Messages access failure.", 
                     info.uid,info.name.c_str());
    }
    return result;
}

bool Task::enrouteMessage(uint64_t msgId)
{
    smsc_log_debug(logger, "enrouteMessage method being called on task '%d/%s' for id=%lld",
                 info.uid,info.name.c_str(), msgId);

    bool result = false;
    try
    {
      result=store.enrouteMessage(msgId);
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. doEnroute(): Messages access failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
    catch (...)
    {
        smsc_log_error(logger, "Task '%d/%s'. doEnroute(): Messages access failure.", 
                     info.uid,info.name.c_str());
    }
    
    return result;
}

void
Task::putToSuspendedMessagesQueue(const Message& suspendedMessage)
{
  smsc_log_info(logger, "Task::putToSuspendedMessagesQueue:::  task '%d/%s': exceeded region bandwidth (regionId=%s), return message with id=%llx to messagesCache",
                info.uid,info.name.c_str(), suspendedMessage.regionId.c_str(), suspendedMessage.id);
  MutexGuard guard(messagesCacheLock);
  _suspendedRegions.insert(suspendedMessage.regionId);
  messagesCache.push_back(suspendedMessage);
}

bool
Task::fetchMessageFromCache(Message& message)
{
  MutexGuard guard(messagesCacheLock);
  std::string regionForPreviouslyFetchedMessage;
  while(_messagesCacheIter != messagesCache.end() ) {
    const Message& fetchedMessage = *_messagesCacheIter;
    if ( regionForPreviouslyFetchedMessage != fetchedMessage.regionId && 
         _suspendedRegions.find(fetchedMessage.regionId) == _suspendedRegions.end() ) {
      message = fetchedMessage; setInProcess(isEnabled());
      _messagesCacheIter = messagesCache.erase(_messagesCacheIter);
      smsc_log_debug(logger, "Task::fetchMessageFromCache::: task '%d/%s', fetched next message [id=%llx,abonent=%s,regionId=%s]",info.uid,info.name.c_str(), message.id, message.abonent.c_str(), message.regionId.c_str());
      return true;
    }
    ++_messagesCacheIter;
    regionForPreviouslyFetchedMessage = fetchedMessage.regionId;
  }
  smsc_log_debug(logger, "Task::fetchMessageFromCache::: cache bypassed, there are no messages meeting search criterion; messagesCache.size = %d", messagesCache.size());
  return false;
}

void
Task::resetSuspendedRegions()
{
  MutexGuard guard(messagesCacheLock);
  smsc_log_debug(logger, "Task::resetSuspendedRegions::: method being called on task '%d/%s'",info.uid,info.name.c_str());
  _messagesCacheIter = messagesCache.begin();
  _suspendedRegions.clear();
}

bool Task::getNextMessage(Message& message)
{
    smsc_log_debug(logger, "getNextMessage method being called on task '%d/%s'",
                   info.uid,info.name.c_str());

    if (!isEnabled())
      return setInProcess(false);

    // selecting from cache
    if ( fetchMessageFromCache(message) )
      return true;

    // Cache is empty here or maybe we have bypassed all cache elements but there are ones with region id values for which suspended condition is true

    if (info.trackIntegrity && !isGenerationSucceeded())
      return setInProcess(false); // for track integrity check that generation finished ok

    time_t currentTime = time(NULL);
    size_t messagesCacheSz;
    {
      MutexGuard guard(messagesCacheLock);
      messagesCacheSz = messagesCache.size();
    }

    if ( messagesCacheSz > 0 ) return false;
    else if (currentTime-lastMessagesCacheEmpty > info.messagesCacheSleep)
      lastMessagesCacheEmpty = currentTime;   // timeout reached, set new sleep timeout & go to fill cache
    else if (bSelectedAll && !isInGeneration()) 
      return setInProcess(false);             // if all selected from DB (on last time) & no generation => return
    smsc_log_info(logger, "Selecting messages from DB. getNextMessage method on task '%d/%s'",
                  info.uid,info.name.c_str());

    // Selecting cache from DB
    try
    {
        int fetched = 0;
        Message fetchedMessage;
        bool haveMsg=false;
        while (fetched < info.messagesCacheSize && (haveMsg=store.getNextMessage(fetchedMessage)))
        {
          MutexGuard guard(messagesCacheLock);
          messagesCache.push_back(fetchedMessage);
          fetched++;
        }
        bSelectedAll = !haveMsg;
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. Messages access failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
    catch (...)
    {
        smsc_log_error(logger, "Task '%d/%s'. Messages access failure.", 
                     info.uid,info.name.c_str());
    }
    
    smsc_log_debug(logger, "Selected %d messages from DB for task '%d/%s'",
                   messagesCache.size(), info.uid,info.name.c_str());

    // selecting from cache
    if ( fetchMessageFromCache(message) )
      return true;
    else {
      lastMessagesCacheEmpty = time(NULL);
      bSelectedAll = true;
      return setInProcess(false);
    }
}

bool Task::isReady(time_t time, bool checkActivePeriod)
{
  if ( !isEnabled() || isFinalizing() || 
       (info.endDate>0 && time>=info.endDate) ||
       (info.validityDate>0 && time>=info.validityDate) )
  { 
    smsc_log_debug(logger, "Task::isReady::: task id =%d/%s,info.endDate=%ld,info.validityDate=%ld,time()=%ld",info.uid,info.name.c_str(),info.endDate,info.validityDate,::time(0));
    if ( !infoSme_T_storageWasDestroyed && !isInProcess() &&
         ((info.endDate>0 && time>=info.endDate) ||
          (info.validityDate>0 && time>=info.validityDate)) )
    {
      destroy_InfoSme_T_Storage();
      infoSme_T_storageWasDestroyed = true;
    }
    return false;
  }

  if (checkActivePeriod) 
  {
      tm dt; localtime_r(&time, &dt);

      if (!info.activeWeekDays.isWeekDay((dt.tm_wday == 0) ? 6:(dt.tm_wday-1)))
          return false;

      if (info.activePeriodStart > 0 && info.activePeriodEnd > 0)
      {
          if (info.activePeriodStart > info.activePeriodEnd) return false;

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

          if (time < apst || time > apet) return false;
      }
  }
  return true;
}

bool Task::insertDeliveryMessage(uint8_t msgState,
                                 const std::string& address,
                                 time_t messageDate,
                                 const std::string& msg)
{
  smsc::sms::Address parsedAddr(address.c_str());
  smsc_log_debug(logger, "Task::insertDeliveryMessage::: try map telephone number [%s] to Region", address.c_str());
  const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(parsedAddr.toString());
  if ( foundRegion )
    smsc_log_debug(logger, "Task::insertDeliveryMessage::: telephone number = %s matches to mask for region with id %s", address.c_str(), foundRegion->getId().c_str());
  else
    throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");

  if (msg.length() > 0)
  {
    Message message;
    message.abonent=address;
    message.message=msg;
    message.date=messageDate;
    message.regionId=foundRegion->getId();
    store.createMessage(messageDate,message);
  }

  return true;
}
bool Task::changeDeliveryMessageInfoByRecordId(uint8_t msgState,
                                               time_t unixTime,
                                               const std::string& recordId)
{
  uint64_t msgId = atol(recordId.c_str());
  smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByRecordId method being called on task '%d/%s' for id=%lld",
                 info.uid,info.name.c_str(), msgId);

  try
  {
    Message msg;
    uint8_t state;
    store.loadMessage(msgId,msg,state);
    store.setMsgState(msgId,DELETED);
    store.createMessage(unixTime,msg,msgState);
    return true;
  }
  catch (std::exception& exc) {
    smsc_log_error(logger, "Task '%d/%s'. changeDeliveryMessageInfoByRecordId(): Messages access failure. "
                   "Details: %s", info.uid,info.name.c_str(), exc.what());
  }
  catch (...) {
    smsc_log_error(logger, "Task '%d/%s'. changeDeliveryMessageInfoByRecordId(): Messages access failure.", 
                   info.uid,info.name.c_str());
  }
    
  return false;
}

static const char* DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_ID  = "%s_FULL_TABLE_SCAN_ID";
static const char* DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL = "FULL_TABLE_SCAN FROM %s";

bool Task::changeDeliveryMessageInfoByCompositCriterion(uint8_t msgState,
                                                        time_t unixTime,
                                                        const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByCompositCriterion method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  int fetched = 0, numOfRowsProcessed=0;

  CsvStore::FullScan fs(store);

  uint8_t state;
  Message msg;
  while (fs.Next(state,msg))
  {
    smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      char recordId[32];
      sprintf(recordId, "%lld", msg.id);

      changeDeliveryMessageInfoByRecordId(msgState,
                                          unixTime,
                                          recordId);
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
    if ( numOfRowsProcessed % 100 == 0 )
      --unixTime;
  }
  return true;
}

bool Task::doesMessageConformToCriterion(uint8_t state,const Message& msg, const InfoSme_T_SearchCriterion& searchCrit)
{
  if ( searchCrit.isSetAbonentAddress() &&
       searchCrit.getAbonentAddress() != msg.abonent ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getAbonentAddress()=%s != rs->getString(3)=%s", searchCrit.getAbonentAddress().c_str(), msg.abonent.c_str());
    return false;
  }
  if ( searchCrit.isSetState() &&
       searchCrit.getState() != state ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getState=%d != rs->getUint8(2)=%d", searchCrit.getState(), state);
    return false;
  }
  if ( searchCrit.isSetFromDate() && 
       searchCrit.getFromDate() > msg.date ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getFromDate()=%ld > rs->getDateTime(4)=%ld", searchCrit.getFromDate(), msg.date);
    return false;
  }
  if ( searchCrit.isSetToDate() && 
       searchCrit.getToDate() < msg.date ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getToDate()=%ld < rs->getDateTime(4)=%ld", searchCrit.getToDate(), msg.date);
    return false;
  }
  return true;
}

bool Task::deleteDeliveryMessageByRecordId(const std::string& recordId)
{
  smsc_log_debug(logger, "Task::deleteDeliveryMessageByRecordId method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  uint64_t msgId = atol(recordId.c_str());
  store.setMsgState(msgId,DELETED);
  return true;
}

bool Task::deleteDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  int fetched = 0, numOfRowsProcessed=0;
  smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion::: search records matching criterion");
  CsvStore::FullScan fs(store);
  uint8_t state;
  Message msg;
  while (fs.Next(state,msg))
  {
    smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      char recordId[32];
      sprintf(recordId, "%lld", msg.id);
      smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion: record found[recordId=%s]",recordId);
      deleteDeliveryMessageByRecordId(recordId);
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }
  return true;

}

struct MessageDescription {
  MessageDescription(uint64_t anId, uint8_t aState, 
                     const std::string& anAbonentAddress,
                     time_t aSendDate,
                     const std::string& aMessage)
    : id(anId), state(aState), abonentAddress(anAbonentAddress), sendDate(aSendDate), message(aMessage){}

  std::string toString()
  {
    std::ostringstream obuf;
    obuf << "id=[" << id 
         << "],state=[" << uint_t(state)
         << "],abonentAddress=[" << abonentAddress
         << "],sendDate=[" << sendDate
         << "],message=[" << message 
         << "]";
    return obuf.str();
  }
  uint64_t id;
  uint8_t state;
  std::string abonentAddress;
  time_t sendDate;
  std::string message;
};

static bool orderByState(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.state < rhs.state ) return true;
  else return false;
}

static bool reverseOrderByState(MessageDescription& lhs, MessageDescription& rhs)
{  
  if ( lhs.state > rhs.state ) return true;
  else return false;
}

static bool orderByAbonent(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.abonentAddress < rhs.abonentAddress ) return true;
  else return false;
}

static bool reverseOrderByAbonent(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.abonentAddress > rhs.abonentAddress ) return true;
  else return false;
}

static bool orderBySendDate(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.sendDate < rhs.sendDate ) return true;
  else return false;
}

static bool reverseOrderBySendDate(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.sendDate > rhs.sendDate ) return true;
  else return false;
}

Array<std::string>
Task::selectDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  typedef std::list<MessageDescription> MessagesList_t;
  MessagesList_t messagesList;

  int fetched = 0;
  size_t msgLimit=0;
  if ( searchCrit.isSetMsgLimit() )
    msgLimit = searchCrit.getMsgLimit();

  size_t fetchedCount=0, numOfRowsProcessed=0;
  CsvStore::FullScan fs(store);
  uint8_t state;
  Message msg;
  while (fs.Next(state,msg))
  {
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      if ( msgLimit && fetchedCount++ >= msgLimit ) continue;
      smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message [%llx,%d,%s,%lld,%s] into messagesList",
                     msg.id, state, msg.abonent.c_str(), msg.date, msg.message.c_str());
      messagesList.push_back(MessageDescription(msg.id,
                                                state,
                                                msg.abonent,
                                                msg.date,
                                                msg.message));
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }

  if ( searchCrit.isSetOrderByCriterion() )
  {
    smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: try sort taskMessages");
    bool (*predicate)(MessageDescription& lhs, MessageDescription& rhs) = NULL;

    if ( searchCrit.getOrderByCriterion() == "state" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderByState;
      else
        predicate = orderByState;
    } else if ( searchCrit.getOrderByCriterion() == "abonent" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderByAbonent;
      else
        predicate = orderByAbonent;
    } else if ( searchCrit.getOrderByCriterion() == "send_date" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderBySendDate;
      else
        predicate = orderBySendDate;
    }
    if ( predicate ) 
      messagesList.sort(predicate);
  }
  smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion:::  prepare taskMessages");
  Array<std::string> taskMessages;
  for(MessagesList_t::iterator iter=messagesList.begin(); iter!=messagesList.end(); ++iter) {
    std::ostringstream messageBuf;
    messageBuf << iter->id << "|"
               << uint_t(iter->state) << "|"
               << iter->abonentAddress << "|"
               << unixTimeToStringFormat(iter->sendDate) << "|"
               << iter->message;
    smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message=[%s] to taskMessages", messageBuf.str().c_str());
    taskMessages.Push(messageBuf.str());
  }
  std::ostringstream fetchedCountAsStr;
  fetchedCountAsStr << fetchedCount;
  taskMessages.Push(fetchedCountAsStr.str());
  return taskMessages;
}

void
Task::endDeliveryMessagesGeneration()
{
  smsc_log_debug(logger, "Task::endDeliveryMessagesGeneration method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  MutexGuard guard(inGenerationLock);
  bInGeneration = false; bGenerationSuccess = true;
  /*  Manager& configManager = Manager::getInstance();

  configManager.setBool(("InfoSme.Tasks."+info.id+".taskLoaded").c_str(), true);
  configManager.save();*/
  generationEndEvent.Signal();
}

void
Task::destroy_InfoSme_T_Storage()
{
  smsc_log_debug(logger, "Task::destroy_InfoSme_T_Storage method being called on task '%d/%s'", info.uid,info.name.c_str());

  try {
    store.Delete(false);
  } catch (std::exception& ex) {
    smsc_log_info(logger, "Task::destroy_InfoSme_T_Storage::: catch exception [%s] for task '%d/%s'",
                  ex.what(), info.uid,info.name.c_str());
  }
}

static const char * CHANGE_DELIVERY_MESSAGE_BY_ID_STATEMENT_SQL = "UPDATE %s SET MESSAGE = :MSG WHERE ID = :ID";

bool Task::changeDeliveryTextMessageByCompositCriterion(const std::string& newTextMsg,
                                                        const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::changeDeliveryTextMessageByCompositCriterion method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  CsvStore::FullScan fs(store);
  uint8_t state;
  Message msg;
  int fetched = 0, numOfRowsProcessed=0;
  while (fs.Next(state,msg))
  {
    smsc_log_debug(logger, "Task::changeDeliveryTextMessageByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      store.setMsgState(msg.id,DELETED);
      store.createMessage(msg.date,msg,state);
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }
  return true;
}

}}
