
#include "Cleaner.h"

#include <util/debug.h>

namespace smsc { namespace store 
{

using smsc::core::threads::Thread;
using namespace smsc::core::synchronization;

const int SMS_ID_PRELOAD_COUNT = 1000;

/* --------------------- Cleaner implementation -------------------- */ 

Cleaner::Cleaner(Manager& config)
    throw(ConfigException) 
        : Thread(), log(Logger::getInstance("smsc.store.Cleaner")),
            storageDBInstance(0L), storageDBUserName(0L), storageDBUserPassword(0L),
                bStarted(false), bNeedExit(false), cleanerConnection(0),
                    cleanerMinTimeStmt(0), cleanerDeleteStmt(0), cleanerNextIdStmt(0),
                        currentId(0), sequenceId(0)
{
    storageDBInstance = 
        loadDBInstance(config, "MessageStore.Storage.dbInstance");
    storageDBUserName = 
        loadDBUserName(config, "MessageStore.Storage.dbUserName");
    storageDBUserPassword = 
        loadDBUserPassword(config, "MessageStore.Storage.dbUserPassword");
    
    __require__(storageDBInstance && storageDBUserName && storageDBUserPassword);
    
    cleanerConnection = new Connection(
        storageDBInstance, storageDBUserName, storageDBUserPassword);
    
    loadCleanupInterval(config);        // in seconds
    loadCleanupAgeInterval(config);     // in seconds
    loadCleanupAwakeInterval(config);   // in mseconds
}
Cleaner::~Cleaner()
{
    smsc_log_info(log, "Cleaner destruction ...");
    
    this->Stop();
    
    if (storageDBInstance) delete storageDBInstance;
    if (storageDBUserName) delete storageDBUserName;
    if (storageDBUserPassword) delete storageDBUserPassword;

    if (cleanerConnection) delete cleanerConnection;
    
    smsc_log_info(log, "Cleaner destructed !");
}
void Cleaner::Start()
{
    MutexGuard  guard(startLock);

    if (!bStarted)
    {
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
    }
}
void Cleaner::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
    }
}
int Cleaner::Execute()
{
    bool first = true;
    while (!bNeedExit)
    {
        awake.Wait((first) ? 1000:awakeInterval);
        if (bNeedExit) break;
        try 
        {
            cleanup(); first = false;
        } 
        catch (StorageException& exc) 
        {
            awake.Wait(0); first = true;
            smsc_log_error(log, "Exception occurred during archive cleanup : %s",
                      exc.what());
        }
    }
    exited.Signal();
    return 0;
}
void Cleaner::cleanup() 
    throw(StorageException)
{
    MutexGuard  guard(cleanupLock);

    connect();

    time_t toDelete = 0;
    cleanerMinTimeStmt->execute();
    if (indDbTime != OCI_IND_NOTNULL) toDelete = 0;
    else Statement::convertOCIDateToDate(&dbTime, &toDelete);

    time_t toTime = time(0) - ageInterval;

    while (toDelete>0 && toDelete<toTime && !bNeedExit)
    {
        time_t oldDelete = toDelete;
        toDelete += cleanupInterval;
        if (toDelete > toTime) toDelete=toTime;
        Statement::convertDateToOCIDate(&toDelete, &dbTime);
        cleanerDeleteStmt->bind(1, SQLT_ODT, (dvoid *) &(dbTime), 
                                (sb4) sizeof(dbTime));
        cleanerDeleteStmt->execute();

            smsc_log_debug(log, "Archive cleanup: %d rows deleted (from %d to %d)",
                      cleanerDeleteStmt->getRowsAffectedCount(), 
                      oldDelete, toDelete);
        cleanerConnection->commit();
    }
}
void Cleaner::connect() 
    throw(StorageException)
{
    if (!cleanerConnection->isAvailable())
    {
        cleanerConnection->connect(); // should destroy all assigned statements
        
        prepareCleanerNextIdStmt();
        prepareCleanerMinTimeStmt();
        prepareCleanerDeleteStmt();
    }
}

SMSId Cleaner::getNextId()
    throw(StorageException)
{
    MutexGuard  guard(idLock);
    
    if (!currentId || !sequenceId || currentId-sequenceId >= SMS_ID_PRELOAD_COUNT)
    {
        MutexGuard  guard(cleanupLock); // TODO: change lock here
        
        connect();
        cleanerNextIdStmt->check(cleanerNextIdStmt->execute());
        cleanerNextIdStmt->getSMSId(sequenceId);
        currentId = sequenceId;
    }
    
    return ++currentId;
}

static const char* storageNextIdSql = (const char*)
"SELECT NVL(SMS_MSG_SEQ.NEXTVAL, 0) FROM DUAL";
void Cleaner::prepareCleanerNextIdStmt() 
    throw(StorageException)
{
    cleanerNextIdStmt = new GetIdStatement(cleanerConnection, storageNextIdSql, true);
}

static const char* cleanerMinTimeSql = (const char*)
"SELECT MIN(LAST_TRY_TIME) FROM SMS_ARC";
void Cleaner::prepareCleanerMinTimeStmt() 
    throw(StorageException)
{
    cleanerMinTimeStmt = new Statement(cleanerConnection, cleanerMinTimeSql, true);

    cleanerMinTimeStmt->define(1, SQLT_ODT, (dvoid *) &(dbTime), 
                               (sb4) sizeof(dbTime), &indDbTime);
}

static const char* cleanerDeleteSql = (const char*)
"DELETE FROM SMS_ARC WHERE LAST_TRY_TIME<:LT";
void Cleaner::prepareCleanerDeleteStmt() 
    throw(StorageException)
{
    cleanerDeleteStmt = new Statement(cleanerConnection, cleanerDeleteSql, true);
}

const unsigned SMSC_CLEANUP_AGE_INTERVAL_LIMIT   = 365; // days
const unsigned SMSC_CLEANUP_AGE_INTERVAL_DEFAULT = 30;  // days
void Cleaner::loadCleanupAgeInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Cleaner.age");
        if (interval <= 0 || 
            interval > SMSC_CLEANUP_AGE_INTERVAL_LIMIT)
        {
            interval = SMSC_CLEANUP_AGE_INTERVAL_DEFAULT;
            smsc_log_warn(log, "Cleanup age interval for storage is incorrect "
                     "(should be between 1 and %u days) ! "
                     "Config parameter: <MessageStore.Cleaner.age> "
                     "Using default: %u", 
                     SMSC_CLEANUP_AGE_INTERVAL_LIMIT,
                     SMSC_CLEANUP_AGE_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_CLEANUP_AGE_INTERVAL_DEFAULT;
        smsc_log_warn(log, "Cleanup age interval for storage missed "
                 "(it should be between 1 and %u days) ! "
                 "Config parameter: <MessageStore.Cleaner.age> "
                 "Using default: %u", 
                 SMSC_CLEANUP_AGE_INTERVAL_LIMIT,
                 SMSC_CLEANUP_AGE_INTERVAL_DEFAULT);
    }

    ageInterval = interval*3600*24; // in seconds
}

const unsigned SMSC_CLEANUP_AWAKE_INTERVAL_LIMIT = 3600; // seconds
const unsigned SMSC_CLEANUP_AWAKE_INTERVAL_DEFAULT =  5; // seconds
void Cleaner::loadCleanupAwakeInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Cleaner.awake");
        if (interval <= 0 || 
            interval > SMSC_CLEANUP_AWAKE_INTERVAL_LIMIT)
        {
            interval = SMSC_CLEANUP_AWAKE_INTERVAL_DEFAULT;
            smsc_log_warn(log, "Awake interval for storage cleaner is incorrect "
                     "(should be between 1 and %u seconds) ! "
                     "Config parameter: <MessageStore.Cleaner.awake> "
                     "Using default: %u", 
                     SMSC_CLEANUP_AWAKE_INTERVAL_LIMIT,
                     SMSC_CLEANUP_AWAKE_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_CLEANUP_AWAKE_INTERVAL_DEFAULT;
        smsc_log_warn(log, "Awake interval for storage cleaner missed "
                 "(it should be between 1 and %u seconds) ! "
                 "Config parameter: <MessageStore.Cleaner.awake> "
                 "Using default: %u", 
                 SMSC_CLEANUP_AWAKE_INTERVAL_LIMIT,
                 SMSC_CLEANUP_AWAKE_INTERVAL_DEFAULT);
    }

    awakeInterval = interval*1000; // in mseconds
}

const unsigned SMSC_CLEANUP_INTERVAL_LIMIT   = 3600; // seconds
const unsigned SMSC_CLEANUP_INTERVAL_DEFAULT = 60;   // seconds
void Cleaner::loadCleanupInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Cleaner.interval");
        if (interval <= 0 || 
            interval > SMSC_CLEANUP_INTERVAL_LIMIT)
        {
            interval = SMSC_CLEANUP_INTERVAL_DEFAULT;
            smsc_log_warn(log, "Cleanup interval for storage is incorrect "
                     "(should be between 1 and %u seconds) ! "
                     "Config parameter: <MessageStore.Cleaner.interval> "
                     "Using default: %u", 
                     SMSC_CLEANUP_INTERVAL_LIMIT,
                     SMSC_CLEANUP_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_CLEANUP_INTERVAL_DEFAULT;
        smsc_log_warn(log, "Cleanup interval for storage missed "
                 "(it should be between 1 and %u seconds) ! "
                 "Config parameter: <MessageStore.Cleaner.interval> "
                 "Using default: %u", 
                 SMSC_CLEANUP_INTERVAL_LIMIT,
                 SMSC_CLEANUP_INTERVAL_DEFAULT);
    }

    cleanupInterval = interval; // in seconds
}

char* Cleaner::loadDBInstance(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        char* tmp = config.getString(cat);   
        char* instance = new char[strlen(tmp)+1];
        strcpy(instance, tmp);
        return instance;
    } 
    catch (ConfigException& exc) 
    {
        smsc_log_error(log, "DB instance name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
char* Cleaner::loadDBUserName(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        char* tmp = config.getString(cat);   
        char* name = new char[strlen(tmp)+1];
        strcpy(name, tmp);
        return name;
    } 
    catch (ConfigException& exc) 
    {
        smsc_log_error(log, "DB user name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
char* Cleaner::loadDBUserPassword(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        char* tmp = config.getString(cat);   
        char* password = new char[strlen(tmp)+1];
        strcpy(password, tmp);
        return password;
    } 
    catch (ConfigException& exc) 
    {
        smsc_log_error(log, "DB user password wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}

}}


