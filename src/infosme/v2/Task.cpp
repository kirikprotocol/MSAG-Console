#include "Task.h"
#include "InfoSmeAdmin.h"
#include "SQLAdapters.h"
#include "TaskLock.hpp"
#include <time.h>
#include <sstream>
#include <list>
#include "util/sleep.h"
#include "sms/sms.h"
#include "util/vformat.hpp"
#include "util/config/region/Region.hpp"
#include "util/config/region/RegionFinder.hpp"

extern bool isMSISDNAddress(const char* string);

namespace smsc {
namespace infosme {

unsigned Task::stringToTaskId( const char* taskId )
{
    if ( !taskId || !*taskId ) throw ConfigException("taskId is null");
    char* endptr;
    unsigned ret = unsigned(strtoul(taskId,&endptr,10));
    if ( *endptr != '\0') {
        throw ConfigException("taskId '%s' is not fully converted",taskId);
    } else if ( ret == 0 ) {
        throw ConfigException("taskId 0 is forbidden");
    }
    return ret;
}


Task::Task( uint32_t           taskId,
            const std::string& location,
            const TaskInfo&    taskInfo,
            DataSource* _dsOwn,
            ServicesForTask*   finalStateSaver ) :
logger(Logger::getInstance("smsc.infosme.Task")),
formatter(0),
finalStateSaver_(finalStateSaver),
usersCount(0), bFinalizing(false), bSelectedAll(false),
active_(false),
info(taskInfo.uid),
dsOwn(_dsOwn),
prefetched_(false),
messagesInCache_(0),
store(location),
bInProcess(false), bInGeneration(false),
infoSme_T_storageWasDestroyed(false)
{
    smsc_log_debug(logger,"task %u/'%s' ctor",taskInfo.uid,taskInfo.name.c_str());
    if ( ! smsc::core::buffers::File::Exists(location.c_str()) ) {
        smsc_log_debug(logger,"making a directory %s",location.c_str());
        smsc::core::buffers::File::MkDir(location.c_str());
    }
    try {
        setInfo(taskInfo);
    } catch (...) {
        if (formatter) delete formatter;
        throw;
    }
}

Task::~Task()
{
    if (formatter) delete formatter;

/*  
    if (dsOwn)
    {
      char buf[128];
      char idBuf[32];
      sprintf(idBuf,"%d",info.uid);
      sprintf(buf,USER_QUERY_STATEMENT_ID,idBuf);
      dsOwn->closeRegisteredQueries(buf);
    }
    */
    smsc_log_debug(logger,"task %u/'%s' dtor",info.uid,info.name.c_str());
}


void Task::setInfo( const TaskInfo& taskInfo )
{
    MutexGuard mg(lock_);
    if ( usersCount > 1 ) {
        smsc_log_warn(logger,"task %u/'%s' setting info while users=%u (>1)",
                      taskInfo.uid, taskInfo.name.c_str(), unsigned(usersCount));
    } else {
        smsc_log_debug(logger,"task %u/'%s' setting new info", taskInfo.uid, taskInfo.name.c_str() );
    }

    if ( taskInfo.enabled ) {
        store.Init();
    } else {
        active_ = false;
    }
    info = taskInfo;

    // needed to reset inprocess flag
    setEnabled(taskInfo.enabled);

    if ( ! formatter && !info.msgTemplate.empty() ) {
        formatter = new OutputFormatter(info.msgTemplate.c_str());
    }
    trackIntegrity(true, true); // delete flag & generated messages
}


std::string Task::toString() const
{
    char buf[100];
    snprintf(buf,sizeof(buf),"%u/'%s' prio=%u users=%u ena/proc=%u/%u",
             info.uid,info.name.c_str(),
             info.priority, usersCount, info.enabled, store.isProcessed() );
    return buf;
}


void Task::finalize()
{
  smsc_log_error(logger, "Task::finalize taskId=%d",getId());
  {
    MutexGuard guard(finalizingLock);
    bFinalizing = true;
  }
  endGeneration();
}


bool Task::destroy()
{
  smsc_log_error(logger, "Task::destroy::: taskId=%d",getId());
  finalize();
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
  // delete this;
  return result;
}

bool Task::shutdown()
{
  smsc_log_error(logger, "Task::shutdown::: taskId=%d",getId());
  finalize();
  bool result = false;
  try 
  { 
    resetWaiting(); 
    result = true;
  } catch (std::exception& exc) 
  {
    smsc_log_error(logger, "Reset waiting failed for task '%d/%s'. Reason: %s",
                   info.uid, info.name.c_str(),exc.what());
  } catch (...) 
  {
    smsc_log_error(logger, "Reset waiting failed for task '%d/%s'. Reason is unknown",
                   info.uid,info.name.c_str());
  }
  // delete this;
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
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. Failed to track task integrity. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
    }
}


void Task::collectStatistics( unsigned& openMsgs )
{
    store.collectStatistics( openMsgs );
}


bool Task::beginGeneration(Statistics* statistics)
{
    smsc_log_debug(logger, "beginGeneration method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

    uint64_t totalGenerated = 0;

    {
        MutexGuard guard(inGenerationMon);
        if (bInGeneration || (info.trackIntegrity && isInProcess())) 
        {
          return false;
        }
        bInGeneration = true;
        info.bGenerationSuccess = false;
    }

    Connection* ownConnection = 0;

    try
    {
        ownConnection = dsOwn->getConnection();
        if (!ownConnection)
            throw Exception("Failed to obtain connection to own data source.");
        if (!formatter)
            throw Exception("Formatter is not set");

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

        typedef std::map<std::string,uint64_t> AbntMap;
        AbntMap abntMap;

        CsvStore::FullScan fs(store);
        {
          Message msg;
          uint8_t state;
          while(fs.Next(state,msg))
          {
            if(state<ENROUTE)
            {
              abntMap.insert(AbntMap::value_type(msg.abonent,msg.id));
            }
          }
        }

        int uncommited = 0;
        time_t sendTime=time(NULL)+60;
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
            smsc::sms::Address addr=abonentAddress;
            formatter->format(message, getAdapter, context);
            if (message.length() > 0)
            {
              if (info.replaceIfPresent)
              {
                AbntMap::iterator it=abntMap.find(addr.toString());
                if(it!=abntMap.end())
                {
                  store.setMsgState(it->second,DELETED);
                }
              }
              Message msg;
              msg.abonent=addr.toString().c_str();
              msg.message=message;
              const int foundRegion = finalStateSaver_->findRegionByAddress( addr.toString().c_str() );
              if ( foundRegion < -1 ) {
                  throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");
              }
                smsc_log_debug(logger, "Task::beginGeneration::: abonent = %s matches to mask for region with id %d", addr.toString().c_str(), foundRegion );

              msg.regionId = foundRegion;

              store.createMessage(sendTime,msg);

              if (statistics)
              {
                statistics->incGenerated(info.uid, 1);
              }

              totalGenerated++;
              if(totalGenerated%500==0)
              {
                sendTime++;
              }
            }
          }
        }
        
        {
          MutexGuard guard(inGenerationMon);
          if (info.trackIntegrity && !bInGeneration)
          {
            info.bGenerationSuccess = false;
          }
          else
          {
            info.bGenerationSuccess = true;
          }
        }

        trackIntegrity(true, false); // delete flag only
    }
    catch (std::exception& exc)
    {
        smsc_log_error(logger, "Task '%d/%s'. Messages generation process failure. "
                     "Details: %s", info.uid,info.name.c_str(), exc.what());
        
        trackIntegrity(true, true); // delete flag & generated messages
        MutexGuard guard(inGenerationMon);
        info.bGenerationSuccess = false;
    }
    
    if (ownConnection)
    {
      dsOwn->freeConnection(ownConnection);
    }
    
    {
      MutexGuard guard(inGenerationMon);
      bInGeneration = false;
      inGenerationMon.notify();
    }
    store.closeAllFiles();
    return (info.bGenerationSuccess && totalGenerated > 0);
}

void Task::endGeneration()
{
    smsc_log_debug(logger, "endGeneration method being called on task '%d/%s' (%s)",
                 info.uid,info.name.c_str(),bInGeneration?"true":"false");
    MutexGuard guard(inGenerationMon);
    if (!bInGeneration)
    {
      return;
    }
    bInGeneration = false;
    inGenerationMon.wait();
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
    smsc_log_debug(logger, "retryMessage method being called on task '%d/%s' for id=%llx",
                 info.uid,info.name.c_str(), msgId);

    bool result = false;
    time_t now=time(NULL);
    if(nextTime<now+60)
    {
      nextTime=now+60;
    }
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

bool Task::finalizeMessage(uint64_t msgId, MessageState state, int smppStatus )
{   
    if (state == NEW || state == WAIT || state == ENROUTE) {
        smsc_log_warn(logger, "Invalid state=%d to finalize message on task '%d/%s' for id=%llx",
                      state, info.uid,info.name.c_str(), msgId);
        return false;
    } else {
        smsc_log_debug(logger, "finalizeMessage(%d) method being called on task '%d/%s' for id=%llx",
                       state, info.uid,info.name.c_str(), msgId);
    }

    bool result = false;
    try
    {
      time_t now = time(0);
      Message msg;
      if (info.keepHistory)
      {
        store.finalizeMsg(msgId,now,state, info.saveFinalState ? &msg : 0);
      }
      else
      {
        store.setMsgState(msgId,DELETED, info.saveFinalState ? &msg : 0);
      }

      if ( info.saveFinalState ) {
          finalStateSaver_->saveFinalState(now,info,msg,state,smppStatus,store.isProcessed());
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
    smsc_log_debug(logger, "enrouteMessage method being called on task '%d/%s' for id=%llx",
                 info.uid,info.name.c_str(), msgId);

    bool result = false;
    try
    {
      store.enrouteMessage(msgId);
      result=true;
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

void Task::putToSuspendedMessagesQueue( const Message& suspendedMessage )
{
    smsc_log_info(logger, "task %u/'%s' message %llx region %d is suspended",
                  info.uid, info.name.c_str(),
                  suspendedMessage.id,
                  suspendedMessage.regionId );
    MutexGuard mg(lock_);
    doSuspendMessage(suspendedMessage);
}


bool Task::prefetchMessage( time_t now, int regionId )
{
    MutexGuard mg(lock_);

    if ( !active_ ) return false;
    if ( prefetched_ ) {
        if ( prefetch_.regionId == regionId ) {
            if ( prefetch_.date <= now ) {
                smsc_log_debug(logger,"task %u/'%s' message %llx for reg=%d is already prefetched",
                               info.uid,info.name.c_str(),prefetch_.id,regionId);
                return true;
            } else {
                smsc_log_debug(logger,"task %u/'%s' prefetched message %llx for reg=%d is in the future",
                               info.uid,info.name.c_str(),prefetch_.id,regionId);
                return false;
            }
        }
        doSuspendMessage(prefetch_);
        prefetched_ = false;
    }

    RegionMap::iterator iter = regionMap_.find(regionId);
    if ( iter != regionMap_.end() ) {
        MessageList& list = iter->second;
        if ( !list.empty() ) {
            Message& msg = list.front();
            if ( msg.date <= now ) {
                prefetch_ = msg;
                list.pop_front();
                --messagesInCache_;
                smsc_log_debug(logger,"task %u/'%s' message %llx for reg=%d prefetched from list, left size=%u",
                               info.uid,info.name.c_str(),prefetch_.id,regionId,unsigned(list.size()));
                return (prefetched_ = true);
            }
        }
    }

    smsc_log_debug(logger,"task %u/'%s' taking from store for region=%d",info.uid,info.name.c_str(),regionId);
    assert(!prefetched_);
    while ( store.getNextMessage(prefetch_) ) {

        prefetched_ = true;

        if ( prefetch_.date > now ) {
            // do not create cache entry, simply remember that message is prefetched
            return false;
        }

        // msg date is ok

        if ( prefetch_.regionId == regionId ) {
            // matched!
            return true;
        }
        doSuspendMessage(prefetch_);
        prefetched_ = false;
    }
    return false;
}


void Task::getPrefetched( Message& msg )
{
    MutexGuard mg(lock_);
    if ( !prefetched_ ) throw smsc::util::Exception("logic problem in task %u: msg is not prefetched",info.uid);
    prefetched_ = false;
    smsc_log_debug(logger,"task %u/'%s' message %llx, region %d, abonent %s fetched",
                   info.uid, info.name.c_str(), prefetch_.id,
                   prefetch_.regionId, prefetch_.abonent.c_str() );
    msg = prefetch_;
}


void Task::checkActivity( time_t now, tm& loctm )
{
    bool newval = false;
    char buf[80];
    const char* what = "";
    MutexGuard mg(lock_);
    do {
        if ( !info.enabled ) {
            what = "disabled";
            break;
        }
        if ( info.trackIntegrity && (bInGeneration || !info.bGenerationSuccess) ) {
            what = "trackInt & genFailed";
            break;
        }

        if ( info.endDate>0 && now>=info.endDate ) {
            if (active_ && logger->isDebugEnabled()) {
                sprintf(buf,"out of endDate=%ld, now=%ld",long(info.endDate),long(now));
                what = buf;
            }
            break;
        }
        if ( info.validityDate>0 && now >= info.validityDate ) {
            if (active_ && logger->isDebugEnabled()) {
                sprintf(buf,"out of validityDate=%ld, now=%ld",long(info.validityDate),long(now));
                what = buf;
            }
            break;
        }

        // checking the storage
        if ( !prefetched_ && (messagesInCache_ == 0) && store.isProcessed() ) {
            what = "storage is processed";
            break;
        }

        const int locDay = (loctm.tm_wday + 6) % 7;
        if ( !info.activeWeekDays.isWeekDay(locDay) ) {
            if ( active_ && logger->isDebugEnabled() ) {
                sprintf(buf,"not active weekday: wDay=0x%x, locDay=%u",
                        unsigned(info.activeWeekDays.weekDays),locDay);
                what = buf;
            }
            break;
        }

        if ( info.activePeriodStart > 0 && info.activePeriodEnd > 0 ) {

            const time_t daytime = (loctm.tm_hour*60+loctm.tm_min)*60+loctm.tm_sec;

            if ( info.activePeriodStart < info.activePeriodEnd && 
                 ( daytime < info.activePeriodStart ||
                   daytime > info.activePeriodEnd ) ) {
                if ( active_ && logger->isDebugEnabled() ) {
                    sprintf(buf,"daytime=%ld is out of activePeriod [%ld,%ld]",
                            long(daytime),
                            long(info.activePeriodStart),
                            long(info.activePeriodEnd));
                    what = buf;
                }
                break;
            }

            if ( info.activePeriodStart > info.activePeriodEnd &&
                 ( daytime < info.activePeriodStart &&
                   daytime > info.activePeriodEnd ) ) {
                if ( active_ && logger->isDebugEnabled() ) {
                    sprintf(buf,"daytime=%ld is out of activePeriod [%ld,%ld]",
                            long(daytime),
                            long(info.activePeriodStart),
                            long(info.activePeriodEnd));
                    what = buf;
                }
                break;
            }
        } // if there is active period set

        newval = true;

    } while ( false );
    if ( logger->isDebugEnabled() && ( newval != active_ ) ) {
        smsc_log_debug(logger,"%u/'%s' change act=%u: %s",info.uid,info.name.c_str(),newval,what);
    }
    active_ = newval;
}


bool Task::insertDeliveryMessage(uint8_t msgState,
                                 const std::string& address,
                                 time_t messageDate,
                                 const std::string& msg,
                                 const std::string& userData )
{
  smsc::sms::Address parsedAddr(address.c_str());
  smsc_log_debug(logger, "Task::insertDeliveryMessage::: try map telephone number [%s] to Region", address.c_str());
  const int foundRegion = finalStateSaver_->findRegionByAddress(parsedAddr.toString().c_str());
  if ( foundRegion < -1 ) {
      throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");
  }
    smsc_log_debug(logger, "Task::insertDeliveryMessage::: telephone number = %s matches to mask for region with id %d", address.c_str(), foundRegion );

  if (! msg.empty())
  {
    time_t now=time(NULL);
    if(messageDate<now+60)
    {
      messageDate=now+60;
    }
    Message message;
    message.abonent=address;
    message.message=msg;
    message.date=messageDate;
    message.regionId = foundRegion;
    if (!userData.empty()) message.userData = userData;
    store.createMessage(messageDate,message);
  }
  return true;
}

bool Task::changeDeliveryMessageInfoByRecordId(uint8_t msgState,
                                               time_t unixTime,
                                               const std::string& recordId,uint64_t& newMsgId)
{
  uint64_t msgId = atol(recordId.c_str());
  smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByRecordId method being called on task '%d/%s' for id=%llx",
                 info.uid,info.name.c_str(), msgId);

  try
  {
    Message msg;
    uint8_t state;
    time_t now=time(NULL);
    if(unixTime<now+60)
    {
      unixTime=now+60;
    }
    store.loadMessage(msgId,msg,state);
    store.setMsgState(msgId,DELETED);
    newMsgId=store.createMessage(unixTime,msg,msgState);
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

  std::set<uint64_t> processedMsgs;

  uint8_t state;
  Message msg;
  while (fs.Next(state,msg))
  {
    smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      if(processedMsgs.find(msg.id)!=processedMsgs.end())
      {
        continue;
      }
      char recordId[32];
      sprintf(recordId, "%lld", msg.id);

      uint64_t newMsgId;
      changeDeliveryMessageInfoByRecordId(msgState,
                                          unixTime,
                                          recordId,
                                          newMsgId);
      processedMsgs.insert(newMsgId);
    }
    numOfRowsProcessed++;
    if ( numOfRowsProcessed % 5000 == 0 )
    {
        smsc::util::millisleep(1);
    }
    if ( numOfRowsProcessed % 100 == 0 )
    {
        unixTime++;
    }
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
  try {
      store.setMsgState(msgId,DELETED);
  } catch ( std::exception& e ) {
      smsc_log_error(logger, "Task '%d/%s'. Cannot delete msg #%llx: %s",
                     info.uid, info.name.c_str(), msgId, e.what());
  } catch (...) {
      smsc_log_error(logger, "Task '%d/%s'. Cannot delete msg #%llx.",
                     info.uid, info.name.c_str(), msgId);
  }
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
    //std::ostringstream obuf;
    /*obuf << "id=[" << id 
         << "],state=[" << uint_t(state)
         << "],abonentAddress=[" << abonentAddress
         << "],sendDate=[" << sendDate
         << "],message=[" << message 
         << "]";
    return obuf.str();
    */
    std::string rv=smsc::util::format("id=[%d],state=[%d],abonentAddress=[%s],sendDate=[%d],message=[%s]",id,state,abonentAddress.c_str(),sendDate,message.c_str());
    return rv;
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
      smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message [%llx,%d,%s,%ld,%s] into messagesList",
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
  for(MessagesList_t::iterator iter=messagesList.begin(); iter!=messagesList.end(); ++iter)
  {
    std::string msgBuf=smsc::util::format("%lld|%d|%s|%s|%s",iter->id,iter->state,iter->abonentAddress.c_str(),unixTimeToStringFormat(iter->sendDate).c_str(),iter->message.c_str());
    /*messageBuf << iter->id << "|"
               << uint_t(iter->state) << "|"
               << iter->abonentAddress << "|"
               << unixTimeToStringFormat(iter->sendDate) << "|"
               << iter->message;*/
    smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message=[%s] to taskMessages", msgBuf.c_str());
    taskMessages.Push(msgBuf);
  }
  //std::ostringstream fetchedCountAsStr;
  //fetchedCountAsStr << fetchedCount;
  taskMessages.Push(smsc::util::format("%d",fetchedCount));//fetchedCountAsStr.str());
  return taskMessages;
}

void
Task::endDeliveryMessagesGeneration()
{
  smsc_log_debug(logger, "Task::endDeliveryMessagesGeneration method being called on task '%d/%s'",
                 info.uid,info.name.c_str());

  MutexGuard guard(inGenerationMon);
  bInGeneration = false;
  info.bGenerationSuccess = true;
  store.closeAllFiles();
  /*  Manager& configManager = Manager::getInstance();

  configManager.setBool(("InfoSme.Tasks."+info.id+".taskLoaded").c_str(), true);
  configManager.save();*/
  inGenerationMon.notify();
}

void
Task::destroy_InfoSme_T_Storage()
{
  if(info.keepHistory)
  {
    return;
  }
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
  time_t date=time(NULL)+60;
  std::set<uint64_t> processedMsgs;
  while (fs.Next(state,msg))
  {
    smsc_log_debug(logger, "Task::changeDeliveryTextMessageByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(state,msg, searchCrit) )
    {
      if(processedMsgs.find(msg.id)!=processedMsgs.end())
      {
        continue;
      }
      try {
          store.setMsgState(msg.id,DELETED);
      } catch ( std::exception& e ) {
          smsc_log_error(logger, "Task '%d/%s'. Cannot delete msg #%llx: %s",
                         info.uid, info.name.c_str(), msg.id, e.what());
      } catch (...) {
          smsc_log_error(logger, "Task '%d/%s'. Cannot delete msg #%llx.",
                         info.uid, info.name.c_str(), msg.id);
      }
      msg.date=date;
      msg.message=newTextMsg;
      processedMsgs.insert(store.createMessage(msg.date,msg,state));
    }
    numOfRowsProcessed++;
    if(numOfRowsProcessed%500==0)
    {
      date++;
    }

    if ( numOfRowsProcessed % 5000 == 0 )
    {
      smsc::util::millisleep(1);
    }
  }
  return true;
}


void Task::doSuspendMessage( const Message& msg )
{
    if (!prefetched_ && &msg != &prefetch_ ) {
        prefetched_ = true;
        prefetch_ = msg;
    } else {
        // looking for a map entry
        RegionMap::iterator iter = regionMap_.lower_bound(msg.regionId);
        if ( iter == regionMap_.end() || iter->first != msg.regionId ) {
            iter = regionMap_.insert
                (iter, RegionMap::value_type
                 ( msg.regionId, RegionMap::mapped_type() ) );
        }
        iter->second.push_front( msg );
        ++messagesInCache_;
    }
}


}}
