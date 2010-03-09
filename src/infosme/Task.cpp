#include "Task.h"
#include "FinalStateSaver.h"
#include "SQLAdapters.h"
#include "TaskLock.hpp"
#include <time.h>
#include <sstream>
#include <list>
#include <deque>
#include "util/sleep.h"
#include "sms/sms.h"
#include "util/vformat.hpp"
#include "util/config/ConfString.h"
#include "util/config/region/Region.hpp"
#include "util/config/region/RegionFinder.hpp"

extern bool isMSISDNAddress(const char* string);

namespace smsc {
namespace infosme {

using namespace smsc::util::config;

class Task::MessageRegionCache
{
public:
    MessageRegionCache();
    ~MessageRegionCache();
    void suspendRegion( const Message& suspendedMessage );
    void resetSuspendedRegions();
    /// NOTE: returns 0 if no messages can be fetched
    uint64_t fetchMessageId();
    time_t fillTime() const;
    size_t fill( time_t now, CsvStore& store, size_t cacheFillSize );
    // bool empty() const;
private:
    typedef std::deque< uint64_t > messagelist_type;
    struct Region {
        std::string       regionId;
        messagelist_type  messages;
        bool              isSuspended;
    };
    typedef smsc::core::buffers::Hash< Region* > regionhash_type;

    smsc::logger::Logger*              log_;
    std::vector< Region* >             regions_;  // owned
    regionhash_type                    regionHash_;
    std::vector< Region* >::iterator   iter_;
    smsc::core::synchronization::Mutex lock_;
    time_t                             fillTime_;
};


Task::MessageRegionCache::MessageRegionCache() :
log_(smsc::logger::Logger::getInstance("task.regc")),
iter_(regions_.begin()),
fillTime_(0)
{
    smsc_log_debug(log_,"region cache is created");
}


Task::MessageRegionCache::~MessageRegionCache()
{
    smsc_log_debug(log_,"region cache is being destroyed");
    for ( std::vector< Region* >::iterator i = regions_.begin(); i != regions_.end(); ++i ) {
        delete *i;
    }
}


void Task::MessageRegionCache::suspendRegion( const Message& suspendedMessage )
{
    MutexGuard mg(lock_);
    Region** ptr = regionHash_.GetPtr( suspendedMessage.regionId.c_str() );
    if ( !ptr ) {
        smsc_log_error(log_,"region %s is not found in cache", suspendedMessage.regionId.c_str() );
        return;
    }
    if ( (*ptr)->isSuspended ) return;
    smsc_log_debug(log_,"suspending region %s",suspendedMessage.regionId.c_str());
    (*ptr)->isSuspended = true;
    (*ptr)->messages.push_front( suspendedMessage.id );
}


void Task::MessageRegionCache::resetSuspendedRegions()
{
    MutexGuard mg(lock_);
    for ( std::vector< Region* >::const_iterator i = regions_.begin(); i != regions_.end(); ++i ) {
        (*i)->isSuspended = false;
    }
}


uint64_t Task::MessageRegionCache::fetchMessageId()
{
    MutexGuard mg(lock_);
    if ( regions_.empty() ) return 0;
    std::vector< Region* >::const_iterator start = iter_;
    uint64_t rv = 0;
    do {
        if ( (*iter_)->isSuspended ) {
            smsc_log_debug(log_,"cached region %s is suspended",(*iter_)->regionId.c_str());
        } else if ( (*iter_)->messages.empty() ) {
            // smsc_log_debug(log_,"cached region %s is empty", (*iter_)->regionId.c_str());
        } else {
            rv = (*iter_)->messages.front();
            smsc_log_debug(log_,"message %llx is found in cached region %s",rv,(*iter_)->regionId.c_str());
            (*iter_)->messages.pop_front();
        }
        ++iter_;
        if ( iter_ == regions_.end() ) iter_ = regions_.begin();
    } while ( rv == 0 && iter_ != start );
    return rv;
}


time_t Task::MessageRegionCache::fillTime() const
{
    return fillTime_;
}


size_t Task::MessageRegionCache::fill( time_t now, CsvStore& store, size_t cacheFileSize )
{
    size_t fetched = 0;
    // MutexGuard mg(lock_);
    Message fetchedMessage;
    bool haveMsg = false;
    // smsc_log_debug(log_,"filling cache at %ld, maxSize=%llu",
    // long(now),static_cast<unsigned long long>(cacheFileSize));
    while ( fetched < cacheFileSize && (haveMsg=store.getNextMessage(fetchedMessage)) ) {
        MutexGuard mg(lock_);
        fillTime_ = now;
        Region** ptr = regionHash_.GetPtr(fetchedMessage.regionId.c_str());
        if ( ! ptr ) {
            // we have to add a new region
            // smsc_log_debug(log_,"a new region %s is created",fetchedMessage.regionId.c_str());
            Region* r = new Region;
            r->regionId = fetchedMessage.regionId;
            r->isSuspended = false;
            const size_t dist = iter_ - regions_.begin();
            regions_.push_back( r );
            iter_ = regions_.begin() + dist;
            regionHash_.Insert(r->regionId.c_str(),r);
            r->messages.push_back(fetchedMessage.id);
        } else {
            (*ptr)->messages.push_back(fetchedMessage.id);
        }
        ++fetched;
    }
    return fetched;
}

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


Task::Task( ConfigView* config,
            uint32_t taskId,
            std::string location, 
            DataSource* _dsOwn,
            FinalStateSaver* finalStateSaver )
    : logger(Logger::getInstance("smsc.infosme.Task")), formatter(0),
      finalStateSaver_(finalStateSaver),
      usersCount(0), bFinalizing(false), bSelectedAll(false), dsOwn(_dsOwn),store(location),
      bInProcess(false), bInGeneration(false), bGenerationSuccess(true),
      infoSme_T_storageWasDestroyed(false),
// lastMessagesCacheEmpty(0), 
messageCache_(new MessageRegionCache),
currentPriorityFrameCounter(0)
{
    try {
  smsc_log_debug(logger,"task %u ctor", taskId);

  if ( ! smsc::core::buffers::File::Exists(location.c_str()) ) {
      smsc_log_debug(logger,"making a directory %s",location.c_str());
      smsc::core::buffers::File::MkDir(location.c_str());
  }

  init(config, taskId);


  formatter = new OutputFormatter(info.msgTemplate.c_str());
  trackIntegrity(true, true); // delete flag & generated messages
    } catch (...) {
        if (formatter) delete formatter;
        if (messageCache_) delete messageCache_;
        throw;
    }
}

Task::~Task()
{
    if (formatter) delete formatter;
    if (messageCache_) delete messageCache_;

    smsc_log_debug(logger,"task %u/'%s' dtor",info.uid,info.name.c_str());
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
}

void Task::init(ConfigView* config, uint32_t taskId)
{
  __require__(config);

  const int MAX_PRIORITY_VALUE = 1000;

  info.uid = taskId;
  try { info.name = ConfString(config->getString("name")).str(); } catch (...) {}
  info.enabled = config->getBool("enabled");
  if(info.enabled)
  {
    store.Init();
  }
  
  info.priority = config->getInt("priority");
  if (info.priority <= 0 || info.priority > MAX_PRIORITY_VALUE)
      throw ConfigException("Task priority should be positive and less than %d.", 
                            MAX_PRIORITY_VALUE);
  try {
    info.address = ConfString(config->getString("address")).str();
  }
  catch (...)
  { 
      smsc_log_warn(logger, "<address> parameter missed for task '%d'. "
                            "Using global definitions", info.uid);
      info.address = "";
  }
    if ( info.address.size() > 0 ) {
        smsc::sms::Address a;
        if ( !info.convertMSISDNStringToAddress(info.address.c_str(),a) ) {
            throw ConfigException("task %u: \"address\" parameter '%s' is wrong",
                                  info.uid,info.address.c_str());
        }
    }

  info.retryOnFail = config->getBool("retryOnFail");
  info.replaceIfPresent = config->getBool("replaceMessage");
  info.transactionMode = config->getBool("transactionMode");
  info.trackIntegrity = config->getBool("trackIntegrity");
  info.keepHistory = config->getBool("keepHistory");
  try {
      info.saveFinalState = config->getBool("saveFinalState");
  } catch ( std::exception& e ) {
  }
  try
  {
    info.flash = config->getBool("flash");
  } catch(std::exception& e)
  {
  }
  info.endDate = parseDateTime(ConfString(config->getString("endDate")).c_str());

  /*
  !!TODO!!??
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

  info.retryPolicy = ConfString(config->getString("retryPolicy")).str();
  if (info.retryOnFail && info.retryPolicy.length()==0)
      throw ConfigException("Task retry time specified incorrectly."); 
  info.validityPeriod = parseTime(ConfString(config->getString("validityPeriod")).c_str());
  info.validityDate = parseDateTime(ConfString(config->getString("validityDate")).c_str());
  if (info.validityPeriod <= 0 && info.validityDate <= 0)
      throw ConfigException("Message validity period/date specified incorrectly.");
  info.activePeriodStart = parseTime(ConfString(config->getString("activePeriodStart")).c_str());
  info.activePeriodEnd = parseTime(ConfString(config->getString("activePeriodEnd")).c_str());
  if ((info.activePeriodStart < 0 && info.activePeriodEnd >= 0) ||
      (info.activePeriodStart >= 0 && info.activePeriodEnd < 0)/* ||
      (info.activePeriodStart >= 0 && info.activePeriodEnd >= 0 && //remove by request
       info.activePeriodStart >= info.activePeriodEnd)*/)
      throw ConfigException("Task active period specified incorrectly."); 
  
  std::string awd;
  try { awd = ConfString(config->getString("activeWeekDays")).str(); }
  catch (...) { 
      smsc_log_warn(logger, "<activeWeekDays> parameter missed for task '%d'. "
                            "Using default: Mon,Tue,Wed,Thu,Fri", info.uid);
      info.activeWeekDays.weekDays = 0x7c; awd.clear();
  }
  if (!awd.empty()) {
      if (!info.activeWeekDays.setWeekDays(awd.c_str()))
          throw ConfigException("Task active week days set listed incorrectly."); 
  }
  else info.activeWeekDays.weekDays = 0;

  if (dsOwn != 0)
  {
      const std::string query_sql = ConfString(config->getString("query")).str();
      if (query_sql.empty())
          throw ConfigException("Sql query for task empty or wasn't specified.");
      info.querySql = query_sql;
      const std::string msg_template = ConfString(config->getString("template")).str();
      if (msg_template.empty())
          throw ConfigException("Message template for task empty or wasn't specified.");
      info.msgTemplate = msg_template;
  }
  info.svcType = "";
  if (info.replaceIfPresent)
  {
      try         { info.svcType = ConfString(config->getString("svcType")).str(); }
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
  try { info.useDataSm = config->getBool("useDataSm"); } catch (...) { info.useDataSm = false; } 
  try { info.useUssdPush = config->getBool("useUssdPush"); }
  catch (...) { info.useUssdPush = false; }
    if ( info.useUssdPush ) {
        // overriding things
        info.transactionMode = true;
        // info.flash = false;
        info.useDataSm = false;
    }
}

void Task::update(ConfigView *config)
{
  const int MAX_PRIORITY_VALUE = 1000;

  TaskInfo newinfo = info;
  try { newinfo.name = ConfString(config->getString("name")).str(); } catch (...) {}
  newinfo.enabled = config->getBool("enabled");
  newinfo.priority = config->getInt("priority");
  if (newinfo.priority <= 0 || newinfo.priority > MAX_PRIORITY_VALUE)
      throw ConfigException("Task priority should be positive and less than %d.", 
                            MAX_PRIORITY_VALUE);
  try {
    newinfo.address = ConfString(config->getString("address")).str();
  }
  catch (...)
  { 
      smsc_log_warn(logger, "<address> parameter missed for task '%d'. "
                            "Using global definitions", newinfo.uid);
      newinfo.address = "";
  }
  newinfo.retryOnFail = config->getBool("retryOnFail");
  newinfo.replaceIfPresent = config->getBool("replaceMessage");
  newinfo.transactionMode = config->getBool("transactionMode");
  newinfo.trackIntegrity = config->getBool("trackIntegrity");
  newinfo.keepHistory = config->getBool("keepHistory");
  try {
      newinfo.saveFinalState = config->getBool("saveFinalState");
  } catch ( std::exception& e ) {
  }
  try
  {
    newinfo.flash = config->getBool("flash");
  } catch(std::exception& e)
  {
  }
  newinfo.endDate = parseDateTime(ConfString(config->getString("endDate")).c_str());

  /*
  !!TODO!!??
  if (newinfo.endDate>0 && time(0)>=newinfo.endDate)
  {
    // preload newinfoSme_T_ storage without index building
    Connection* connection = 0;
    connection = dsInt->getConnection();

    std::string createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
    std::auto_ptr<Statement> statementGuard(connection->createStatement(createTableSql.c_str()));
    Statement* statement = statementGuard.get();
    if (!statement) 
      throw Exception("Failed to create table statement.");
    statement->execute();  

    // and set flag in order to don't try destroy storage needed only for queries execution
    newinfoSme_T_storageWasDestroyed = true;
  }
  */

  newinfo.retryPolicy = ConfString(config->getString("retryPolicy")).c_str();
  if (newinfo.retryOnFail && newinfo.retryPolicy.length()== 0)
      throw ConfigException("Task retry time specified incorrectly."); 
  newinfo.validityPeriod = parseTime(ConfString(config->getString("validityPeriod")).c_str());
  newinfo.validityDate = parseDateTime(ConfString(config->getString("validityDate")).c_str());
  if (newinfo.validityPeriod <= 0 && newinfo.validityDate <= 0)
      throw ConfigException("Message validity period/date specified incorrectly.");
  newinfo.activePeriodStart = parseTime(ConfString(config->getString("activePeriodStart")).c_str());
  newinfo.activePeriodEnd = parseTime(ConfString(config->getString("activePeriodEnd")).c_str());
  if ((newinfo.activePeriodStart < 0 && newinfo.activePeriodEnd >= 0) ||
      (newinfo.activePeriodStart >= 0 && newinfo.activePeriodEnd < 0)/* ||
      (newinfo.activePeriodStart >= 0 && newinfo.activePeriodEnd >= 0 && 
       newinfo.activePeriodStart >= newinfo.activePeriodEnd)*/)
      throw ConfigException("Task active period specified incorrectly."); 

  std::string awd;
  try { awd = ConfString(config->getString("activeWeekDays")).str(); }
  catch (...) { 
      smsc_log_warn(logger, "<activeWeekDays> parameter missed for task '%d'. "
                            "Using default: Mon,Tue,Wed,Thu,Fri", newinfo.uid);
      newinfo.activeWeekDays.weekDays = 0x7c; awd.clear();
  }
  if (!awd.empty()) {
      if (!newinfo.activeWeekDays.setWeekDays(awd.c_str()))
          throw ConfigException("Task active week days set listed incorrectly."); 
  }
  else newinfo.activeWeekDays.weekDays = 0;

  if (dsOwn != 0)
  {
      const std::string query_sql = ConfString(config->getString("query")).str();
      if (query_sql.empty())
          throw ConfigException("Sql query for task empty or wasn't specified.");
      newinfo.querySql = query_sql;
      const std::string msg_template = ConfString(config->getString("template")).str();
      if (msg_template.empty())
          throw ConfigException("Message template for task empty or wasn't specified.");
      newinfo.msgTemplate = msg_template;
  }
  newinfo.svcType = "";
  if (newinfo.replaceIfPresent)
  {
      try         { newinfo.svcType = ConfString(config->getString("svcType")).str(); }
      catch (...) { newinfo.svcType = "";}
  }

  newinfo.dsTimeout = 0;
  try { newinfo.dsTimeout = config->getInt("dsTimeout"); } catch(...) {}
  if (newinfo.dsTimeout < 0) newinfo.dsTimeout = 0;

  newinfo.dsUncommitedInProcess = 1;
  try { newinfo.dsUncommitedInProcess = config->getInt("uncommitedInProcess"); } catch(...) {}
  if (newinfo.dsUncommitedInProcess < 0) newinfo.dsUncommitedInProcess = 1;
  newinfo.dsUncommitedInGeneration = 1;
  try { newinfo.dsUncommitedInGeneration = config->getInt("uncommitedInGeneration"); } catch(...) {}
  if (newinfo.dsUncommitedInGeneration < 0) newinfo.dsUncommitedInGeneration = 1;
  newinfo.messagesCacheSize = 100;
  try { newinfo.messagesCacheSize = config->getInt("messagesCacheSize"); } catch(...) {}
  if (newinfo.messagesCacheSize <= 0) newinfo.messagesCacheSize = 100;
  newinfo.messagesCacheSleep = 1;
  try { newinfo.messagesCacheSleep = config->getInt("messagesCacheSleep"); } catch(...) {}
  if (newinfo.messagesCacheSleep <= 0) newinfo.messagesCacheSleep = 1;
  if(!info.enabled && newinfo.enabled)
  {
    store.Init();
  }
  info=newinfo;
  {
    MutexGuard mg(inGenerationMon);
    if(!bInGeneration)
    {
      OutputFormatter* newFormatter = new OutputFormatter(info.msgTemplate.c_str());
      delete formatter;
      formatter=newFormatter;
    }
  }
}


void Task::doFinalization()
{
  smsc_log_error(logger, "Task::doFinalization::: taskId=%d",getId());
  {
    MutexGuard guard(finalizingLock);
    bFinalizing = true;
  }
  endGeneration();
  while (true)
  {
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
  doFinalization();
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
    catch (std::exception& exc)
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
        MutexGuard guard(inGenerationMon);
        if (bInGeneration || (info.trackIntegrity && isInProcess())) 
        {
          return false;
        }
        bInGeneration = true;
        bGenerationSuccess = false;
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
              const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(addr.toString());
              if ( foundRegion )
                smsc_log_debug(logger, "Task::beginGeneration::: abonent = %s matches to mask for region with id %s", addr.toString().c_str(), foundRegion->getId().c_str());
              else
                throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");

              msg.regionId=foundRegion->getId();

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
            bGenerationSuccess = false;
          }
          else
          {
            bGenerationSuccess = true;
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
        bGenerationSuccess = false;
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
    return (bGenerationSuccess && totalGenerated > 0);
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
          finalStateSaver_->save(now,info,msg,state,smppStatus,store.isProcessed());
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

void
Task::putToSuspendedMessagesQueue(const Message& suspendedMessage)
{
  smsc_log_info(logger, "Task::putToSuspendedMessagesQueue:::  task '%d/%s': exceeded region bandwidth (regionId=%s), return message with id=%llx to messagesCache",
                info.uid, info.name.c_str(), suspendedMessage.regionId.c_str(), suspendedMessage.id);
    messageCache_->suspendRegion( suspendedMessage );
}


void Task::resetSuspendedRegions()
{
    // smsc_log_debug(logger, "Task::resetSuspendedRegions::: method being called on task '%d/%s'",info.uid,info.name.c_str());
    messageCache_->resetSuspendedRegions();
}

bool Task::getNextMessage(Message& message)
{
    if (!isEnabled())
      return setInProcess(false);

    // smsc_log_debug(logger, "getNextMessage method being called on task '%d/%s', ena/fin/inProc/inGen/genOk=%u/%u/%u/%u/%u",
    // info.uid,info.name.c_str(),
    // info.enabled,bFinalizing,bInProcess,bInGeneration,bGenerationSuccess);

    do {
        // selecting from cache (and from underlying store)
        uint64_t msgid = messageCache_->fetchMessageId();
        if ( msgid == 0 ) break;
        try {
            uint8_t state;
            store.loadMessage(msgid,message,state);
            smsc_log_debug(logger,"fetch msgId=%llx from cache",msgid);
            return setInProcess(true);
        } catch ( std::exception& e ) {
            smsc_log_error(logger,"Task '%d/%s'. getNextMsg(%llx): Message access failure: %s",
                           info.uid, info.name.c_str(), msgid, e.what() );
        } catch (...) {
            smsc_log_error(logger,"Task '%d/%s'. getNextMsg(%llx): Message access failure.",
                           info.uid, info.name.c_str(), msgid );
        }
    } while ( true );

    // Cache is empty here or maybe we have bypassed all cache elements but there are ones with region id values for which suspended condition is true

    if (info.trackIntegrity && !isGenerationSucceeded()) {
        smsc_log_debug(logger,"trackIntegrity and generation not succeded");
        return setInProcess(false); // for track integrity check that generation finished ok
    }

    time_t currentTime = time(NULL);

    // if ( !messageCache_->empty() ) return false;
    /*
    if (currentTime-messageCache_->fillTime() > info.messagesCacheSleep) {
       // lastMessagesCacheEmpty = currentTime;   // timeout reached, set new sleep timeout & go to fill cache
    } else if (bSelectedAll && !isInGeneration()) {
        smsc_log_debug(logger,"selectedAll and not in generation");
        return setInProcess(false);             // if all selected from DB (on last time) & no generation => return
    }
    smsc_log_info(logger, "Selecting messages from DB. getNextMessage method on task '%d/%s'",
                  info.uid,info.name.c_str());
     */

    // Filling cache from DB
    size_t fetched = 0;
    try
    {
        // int fetched = 0;
        fetched = messageCache_->fill(currentTime,store,info.messagesCacheSize);
        bSelectedAll = (fetched != info.messagesCacheSize);
        if ( fetched == 0 && store.isProcessed() ) {
            setInProcess(false);
        }
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
    
    if (fetched >0) {
        smsc_log_debug(logger, "Selected %d messages from DB for task '%d/%s'",
                       unsigned(fetched), info.uid,info.name.c_str());
    }

    // selecting from cache
    do {
        uint64_t msgid = messageCache_->fetchMessageId();
        if ( msgid == 0 ) break;
        try {
            uint8_t state;
            store.loadMessage(msgid,message,state);
            return setInProcess(true);
        } catch ( std::exception& e ) {
            smsc_log_error(logger,"Task '%d/%s'. getNextMsg(%llx): Message access failure: %s",
                           info.uid, info.name.c_str(), msgid, e.what() );
        } catch (...) {
            smsc_log_error(logger,"Task '%d/%s'. getNextMsg(%llx): Message access failure.",
                           info.uid, info.name.c_str(), msgid );
        }
    } while ( true );
    return false;
    // lastMessagesCacheEmpty = time(NULL);
    // bSelectedAll = true;
    // -- db: why bSelectedAll is set here?
    // -- the messages may be not accessible because they are suspended
    // return setInProcess(false);
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
          //if (info.activePeriodStart > info.activePeriodEnd) return false;

          dt.tm_isdst = -1;
          dt.tm_hour = (int)info.activePeriodStart/3600;
          dt.tm_min  = (int)(info.activePeriodStart%3600)/60;
          dt.tm_sec  = (int)(info.activePeriodStart%3600)%60;
          time_t apst = mktime(&dt);

          dt.tm_isdst = -1;
          dt.tm_hour = (int)info.activePeriodEnd/3600;
          dt.tm_min  = (int)(info.activePeriodEnd%3600)/60;
          dt.tm_sec  = (int)(info.activePeriodEnd%3600)%60;
          time_t apet = mktime(&dt);

          if (info.activePeriodStart<info.activePeriodEnd && (time < apst || time > apet)) return false;
          if (info.activePeriodStart>info.activePeriodEnd && (time < apst && time > apet)) return false;
      }
  }
  return true;
}

bool Task::insertDeliveryMessage(uint8_t msgState,
                                 const std::string& address,
                                 time_t messageDate,
                                 const std::string& msg,
                                 const std::string& userData )
{
  smsc::sms::Address parsedAddr(address.c_str());
  smsc_log_debug(logger, "Task::insertDeliveryMessage::: try map telephone number [%s] to Region", address.c_str());
  const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(parsedAddr.toString());
  if ( foundRegion )
    smsc_log_debug(logger, "Task::insertDeliveryMessage::: telephone number = %s matches to mask for region with id %s", address.c_str(), foundRegion->getId().c_str());
  else
    throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");

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
    message.regionId=foundRegion->getId();
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
  bGenerationSuccess = true;
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

}}
