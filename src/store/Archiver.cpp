
#include "Archiver.h"

#include <util/debug.h>

namespace smsc { namespace store 
{

using smsc::core::threads::Thread;
using namespace smsc::core::synchronization;

char* Archiver::loadDBInstance(Manager& config, const char* cat)
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
        log.error("DB instance name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
char* Archiver::loadDBUserName(Manager& config, const char* cat)
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
        log.error("DB user name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
char* Archiver::loadDBUserPassword(Manager& config, const char* cat)
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
        log.error("DB user password wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}

const unsigned SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT = 10000000;
const unsigned SMSC_ARCHIVER_AWAKE_INTERVAL_DEFAULT = 1000;

void Archiver::loadAwakeInterval(Manager& config)
{
    try 
    {
        awakeInterval = 
            (unsigned)config.getInt("MessageStore.Archive.interval");
        if (!awakeInterval || 
            awakeInterval > SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT)
        {
            awakeInterval = SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT;
            log.warn("Maximum timeout interval "
                     "(between Archiver activations) is incorrect "
                     "(should be between 1 and %u m-seconds) ! "
                     "Config parameter: <MessageStore.Archive.interval> "
                     "Using maximum: %u",
                     SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT,
                     SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT);
        }
    } 
    catch (ConfigException& exc) 
    {
        awakeInterval = SMSC_ARCHIVER_AWAKE_INTERVAL_DEFAULT;
        log.warn("Maximum timeout interval "
                 "(between Archiver activations) wasn't specified ! "
                 "Config parameter: <MessageStore.Archive.interval> "
                 "Using default: %u",
                 SMSC_ARCHIVER_AWAKE_INTERVAL_DEFAULT);
    }
}

const unsigned SMSC_ARCHIVER_MAX_FINALIZED_LIMIT = 100000;
const unsigned SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT = 1000;

void Archiver::loadMaxFinalizedCount(Manager& config)
{
    try 
    {
        maxFinalizedCount =
            (unsigned)config.getInt("MessageStore.Archive.finalized");
        if (!maxFinalizedCount || 
            maxFinalizedCount > SMSC_ARCHIVER_MAX_FINALIZED_LIMIT)
        {
            maxFinalizedCount = SMSC_ARCHIVER_MAX_FINALIZED_LIMIT;
            log.warn("Maximum count of finalized messages "
                     "in storage (for Archiver activation) is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.Archive.finalized> "
                     "Using maximum: %u",
                     SMSC_ARCHIVER_MAX_FINALIZED_LIMIT,
                     SMSC_ARCHIVER_MAX_FINALIZED_LIMIT);
        }
    } 
    catch (ConfigException& exc) 
    {
        maxFinalizedCount = SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT;
        log.warn("Maximum count of finalized messages "
                 "in storage (for Archiver activation) wasn't specified ! "
                 "Config parameter: <MessageStore.Archive.finalized> "
                 "Using default: %u",
                 SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT);
    }
}

const unsigned SMSC_ARCHIVER_MAX_UNCOMMITED_LIMIT = 1000;
const unsigned SMSC_ARCHIVER_MAX_UNCOMMITED_DEFAULT = 100;

void Archiver::loadMaxUncommitedCount(Manager& config)
{
    try 
    {
        maxUncommitedCount =
            (unsigned)config.getInt("MessageStore.Archive.uncommited");
        if (!maxUncommitedCount || 
            maxUncommitedCount > SMSC_ARCHIVER_MAX_UNCOMMITED_LIMIT)
        {
            maxUncommitedCount = SMSC_ARCHIVER_MAX_UNCOMMITED_LIMIT;
            log.warn("Maximum count of operations on storage, "
                     "uncommited by Archiver, is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.Archive.uncommited> "
                     "Using maximum: %u",
                     SMSC_ARCHIVER_MAX_UNCOMMITED_LIMIT,
                     SMSC_ARCHIVER_MAX_UNCOMMITED_LIMIT);
        }
    } 
    catch (ConfigException& exc) 
    {
        maxUncommitedCount = SMSC_ARCHIVER_MAX_UNCOMMITED_DEFAULT;
        log.warn("Maximum count of operations on storage, "
                 "uncommited by Archiver, wasn't specified ! "
                 "Config parameter: <MessageStore.Archive.uncommited> "
                 "Using default: %u",
                 SMSC_ARCHIVER_MAX_UNCOMMITED_DEFAULT);
    }
}

bool Archiver::needCleaner(Manager& config)
{
    bool cleanerIsNeeded = false;
    try
    {
        cleanerIsNeeded = config.getBool("MessageStore.Archive.Cleaner.enabled");
    }
    catch (ConfigException& exc)
    {
        cleanerIsNeeded = false;
        log.warn("Config parameter: <MessageStore.Archive.Cleaner.enabled> missed. "
                 "Archive cleaner disabled.");
    }
    return cleanerIsNeeded;
}

Archiver::Archiver(Manager& config) throw(ConfigException) 
    : Thread(), log(Logger::getCategory("smsc.store.Archiver")),
        finalizedCount(0), bStarted(false), cleaner(0), cleanerConnection(0),
        storageDBInstance(0L), storageDBUserName(0L), storageDBUserPassword(0L),
        billingDBInstance(0L), billingDBUserName(0L), billingDBUserPassword(0L),
            storageSelectStmt(0L), storageDeleteStmt(0L), archiveInsertStmt(0L),
            billingCleanIdsStmt(0L), billingInsertStmt(0L), billingLookIdStmt(0L),
            billingPutIdStmt(0L) 
{
    storageDBInstance = 
        loadDBInstance(config, "MessageStore.Storage.dbInstance");
    storageDBUserName = 
        loadDBUserName(config, "MessageStore.Storage.dbUserName");
    storageDBUserPassword = 
        loadDBUserPassword(config, "MessageStore.Storage.dbUserPassword");
    billingDBInstance =
        loadDBInstance(config, "MessageStore.Billing.dbInstance");
    billingDBUserName =
        loadDBUserName(config, "MessageStore.Billing.dbUserName");
    billingDBUserPassword =
        loadDBUserPassword(config, "MessageStore.Billing.dbUserPassword");
    
    loadAwakeInterval(config);
    loadMaxFinalizedCount(config);
    loadMaxUncommitedCount(config);

    __require__(storageDBInstance && storageDBUserName && storageDBUserPassword);
    __require__(billingDBInstance && billingDBUserName && billingDBUserPassword);

    storageConnection = new Connection(
        storageDBInstance, storageDBUserName, storageDBUserPassword);
    
    billingConnection = new Connection(
        billingDBInstance, billingDBUserName, billingDBUserPassword);

    if (needCleaner(config))
    {
        cleanerConnection = new Connection(
            storageDBInstance, storageDBUserName, storageDBUserPassword);
        cleaner = new Cleaner(config, cleanerConnection);
    }
}


Archiver::~Archiver() 
{
    log.info("Archiver destruction ...");
    
    this->Stop();
    
    if (storageDBInstance) delete storageDBInstance;
    if (storageDBUserName) delete storageDBUserName;
    if (storageDBUserPassword) delete storageDBUserPassword;
    if (billingDBInstance) delete billingDBInstance;
    if (billingDBUserName) delete billingDBUserName;
    if (billingDBUserPassword) delete billingDBUserPassword;

    if (storageConnection) delete storageConnection;
    if (billingConnection) delete billingConnection;
    
    if (cleaner) delete cleaner;
    if (cleanerConnection) delete cleanerConnection;
    
    log.info("Archiver destructed !");
}

void Archiver::Start() 
    throw(StorageException)
{
    MutexGuard  guard(startLock);

    if (!bStarted)
    {
        startup();
        Thread::Start();
        bStarted = true;
    }
}

void Archiver::Stop() 
{
    MutexGuard  guard(startLock);
    log.info( "Archiver: stopping..." );
    if (bStarted)
    {
        exit.Signal();
        job.Signal();
        exited.Wait();
        bStarted = false;
    }
    log.info( "Archiver: stopped..." );
}

void Archiver::startup()
    throw(StorageException)
{
    MutexGuard  finalizedGuard(finalizedMutex);  

    connect();
    count();
}


void Archiver::connect()
    throw(StorageException)
{
    if (!storageConnection->isAvailable())
    {
        storageConnection->connect();
        
        prepareStorageSelectStmt();
        prepareStorageDeleteStmt();
        prepareArchiveInsertStmt();
    }
    
    if (!billingConnection->isAvailable())
    {
        billingConnection->connect();
        
        prepareBillingPutIdStmt();
        prepareBillingLookIdStmt();
        prepareBillingInsertStmt();
        prepareBillingCleanIdsStmt();
    }
}

void Archiver::incrementFinalizedCount(unsigned count)
{
    MutexGuard  guard(finalizedMutex);  

    if ((unsigned)(finalizedCount += count) >= maxFinalizedCount) 
    {
        if (bStarted && !job.isSignaled()) 
        {
            job.Signal();
            /*__trace2__("Signal sent, Finalized count is : %d !\n", 
                       finalizedCount);*/
        }
    }
}
void Archiver::decrementFinalizedCount(unsigned count)
{
    MutexGuard  guard(finalizedMutex);
    
    if (bStarted)
    {
        finalizedCount -= count;
    }
}

int Archiver::Execute()
{
    log.info("Archiver started !");
    bool first = true;
    do 
    {
        job.Wait((first) ? 0:awakeInterval);
        if (exit.isSignaled()) break;
        try 
        {
            //__trace__("Doing archivation job ...");
            job.Signal();
            archivate(first); first = false; 
            job.Wait(0);
            //__trace__("Archivation job done !");
        } 
        catch (StorageException& exc) 
        {
            first = true; job.Wait(0);
            log.error("Exception occurred during archivation process : %s",
                      exc.what());
        }
    } 
    while (!exit.isSignaled());
    
    exit.Wait(0);
    exited.Signal();
    log.info("Archiver exited !");
    return 0;
}

void Archiver::billing(bool check)
    throw(StorageException)
{
    idCounter = 0;
    if (check)
    {
        billingLookIdStmt->check(billingLookIdStmt->execute());
    }
    
    if (sms.billingRecord && idCounter == 0)
    {
        billingPutIdStmt->check(billingPutIdStmt->execute());
        // TO DO : Isert more code here ! Actual conversion to SMS_BR
        billingInsertStmt->check(billingInsertStmt->execute());
    }
}

void Archiver::archivate(bool first)
    throw(StorageException) 
{
    MutexGuard  guard(processLock);

    connect();

    // do real job here
    unsigned uncommited = 0;
    try 
    {
        if (!first)
        {
            billingCleanIdsStmt->check(billingCleanIdsStmt->execute());
            billingConnection->commit();
        }
        sword status = storageSelectStmt->execute();
        if (status == OCI_NO_DATA) return;
        storageSelectStmt->check(status);
        
        do
        {
            convertStringToAddress(oa, sms.originatingAddress);
            convertStringToAddress(da, sms.destinationAddress);
            convertStringToAddress(dda, sms.dealiasedDestinationAddress);
            
            billing(first);
            
            if (bNeedArchivate == 'Y')
            {
                // insert BLOB body transfer here if needed !
                rebindBody();
                archiveInsertStmt->check(archiveInsertStmt->execute());
            } 
            
            storageDeleteStmt->check(storageDeleteStmt->execute());

            if (++uncommited >= maxUncommitedCount) 
            {
                billingConnection->commit();
                storageConnection->commit();
                decrementFinalizedCount(uncommited);
                uncommited = 0;
            }
            if (exit.isSignaled()) break;
        } 
        while ((status = storageSelectStmt->fetch()) == OCI_SUCCESS ||
               status == OCI_SUCCESS_WITH_INFO);

        if (status != OCI_NO_DATA)
            storageSelectStmt->check(status);
        
        if (uncommited)
        {
            billingConnection->commit();
            storageConnection->commit();
            decrementFinalizedCount(uncommited);
        }
    }
    catch (StorageException& exc)
    {
        try { billingConnection->rollback(); } catch (...) {
            log.error("Failed to rollback on billing connection.");
        }
        try { storageConnection->rollback(); } catch (...) {
            log.error("Failed to rollback on storage connection.");
        }
        
        throw exc;
    }

}

const char* Archiver::billingMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), 0) FROM SMS_IDS";
const char* Archiver::storageMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), 0) FROM SMS_MSG";
const char* Archiver::archiveMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), 0) FROM SMS_ARC";
SMSId Archiver::getMaxUsedId(Connection* connection, const char* sql)
    throw(StorageException)
{
    SMSId maxId;
    
    GetIdStatement maxIdStmt(connection, sql);
    maxIdStmt.check(maxIdStmt.execute());
    maxIdStmt.getSMSId(maxId);
    
    return maxId;
}

SMSId Archiver::getLastUsedId()
    throw(StorageException)
{
    MutexGuard  guard(processLock);
    
    connect();
    
    SMSId storageId = getMaxUsedId(storageConnection, 
                                   Archiver::storageMaxIdSql);
    SMSId archiveId = getMaxUsedId(storageConnection, 
                                   Archiver::archiveMaxIdSql);
    return ((storageId > archiveId) ? storageId : archiveId);
}

const char* Archiver::billingPutIdSql = (const char*)
"INSERT INTO SMS_IDS VALUES (:ID)";
void Archiver::prepareBillingPutIdStmt()
    throw(StorageException)
{
    billingPutIdStmt = new Statement(billingConnection,
                                     Archiver::billingPutIdSql, true);
    
    billingPutIdStmt->bind(1 , SQLT_VNU, (dvoid *) &(smsId),
                           (sb4) sizeof(smsId));
}

const char* Archiver::billingLookIdSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_IDS WHERE ID=:ID";
void Archiver::prepareBillingLookIdStmt()
    throw(StorageException)
{
    billingLookIdStmt = new Statement(billingConnection,
                                     Archiver::billingLookIdSql, true);
    
    billingLookIdStmt->bind(1 , SQLT_VNU, (dvoid *) &(smsId),
                            (sb4) sizeof(smsId));
    billingLookIdStmt->define(1 , SQLT_UIN, (dvoid *) &(idCounter),
                              (sb4) sizeof(idCounter));
}

const char* Archiver::billingCleanIdsSql = (const char*)
"DELETE FROM SMS_IDS";
void Archiver::prepareBillingCleanIdsStmt()
    throw(StorageException)
{
    billingCleanIdsStmt = new Statement(billingConnection,
                                        Archiver::billingCleanIdsSql, true);
}

const char* Archiver::storageCountSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG WHERE NOT ST=:ENROUTE";
void Archiver::count()
    throw(StorageException)
{
    Statement countStmt(storageConnection, Archiver::storageCountSql);

    countStmt.bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
                   (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    countStmt.define(1 , SQLT_UIN, (dvoid *) &(finalizedCount), 
                     (sb4) sizeof(finalizedCount));
    countStmt.check(countStmt.execute());
}

const char* Archiver::storageSelectSql = (const char*)
"SELECT ID, ST, MR, OA, DA, DDA, SVC_TYPE,\
 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,\
 VALID_TIME, SUBMIT_TIME, ATTEMPTS, LAST_RESULT, LAST_TRY_TIME,\
 DR, BR, ARC, BODY, BODY_LEN, ROUTE_ID, SVC_ID, PRTY,\
 SRC_SME_ID, DST_SME_ID, TXT_LENGTH\
 FROM SMS_MSG WHERE NOT ST=:ENROUTE ORDER BY LAST_TRY_TIME ASC";
void Archiver::prepareStorageSelectStmt() throw(StorageException)
{
    storageSelectStmt = new Statement(storageConnection, 
                                      Archiver::storageSelectSql, true);
    
    ub4 i=1;
    storageSelectStmt->define(i++, SQLT_VNU, (dvoid *) &(smsId),
                              (sb4) sizeof(smsId));
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *) &(uState), 
                              (sb4) sizeof(uState));
    storageSelectStmt->define(i++, SQLT_UIN, 
                              (dvoid *) &(sms.messageReference),
                              (sb4) sizeof(sms.messageReference));
    
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (svcType), 
                              (sb4) sizeof(svcType), &indSvc);
    
    storageSelectStmt->define(i++, SQLT_STR, 
                              (dvoid *) (sms.originatingDescriptor.msc),
                              (sb4) sizeof(sms.originatingDescriptor.msc),
                              &indSrcMsc);
    storageSelectStmt->define(i++, SQLT_STR, 
                              (dvoid *) (sms.originatingDescriptor.imsi),
                              (sb4) sizeof(sms.originatingDescriptor.imsi),
                              &indSrcImsi);
    storageSelectStmt->define(i++, SQLT_UIN, 
                              (dvoid *)&(sms.originatingDescriptor.sme),
                              (sb4) sizeof(sms.originatingDescriptor.sme),
                              &indSrcSme);
    storageSelectStmt->define(i++, SQLT_STR, 
                              (dvoid *) (sms.destinationDescriptor.msc),
                              (sb4) sizeof(sms.destinationDescriptor.msc),
                              &indDstMsc);
    storageSelectStmt->define(i++, SQLT_STR, 
                              (dvoid *) (sms.destinationDescriptor.imsi),
                              (sb4) sizeof(sms.destinationDescriptor.imsi),
                              &indDstImsi);
    storageSelectStmt->define(i++, SQLT_UIN, 
                              (dvoid *)&(sms.destinationDescriptor.sme),
                              (sb4) sizeof(sms.destinationDescriptor.sme),
                              &indDstSme);
    storageSelectStmt->define(i++, SQLT_ODT, (dvoid *) &(validTime),
                              (sb4) sizeof(validTime));
    storageSelectStmt->define(i++, SQLT_ODT, (dvoid *) &(submitTime),
                              (sb4) sizeof(submitTime));
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *)&(sms.attempts),
                              (sb4) sizeof(sms.attempts));
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *) &(sms.lastResult),
                              (sb4) sizeof(sms.lastResult));
    storageSelectStmt->define(i++, SQLT_ODT, (dvoid *) &(lastTime),
                              (sb4) sizeof(lastTime), &indLastTime);
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *) &(sms.deliveryReport),
                              (sb4) sizeof(sms.deliveryReport));
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *) &(sms.billingRecord),
                              (sb4) sizeof(sms.billingRecord));
    storageSelectStmt->define(i++, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
                              (sb4) sizeof(bNeedArchivate));
    storageSelectStmt->define(i++, SQLT_BIN, (dvoid *) (bodyBuffer), 
                              (sb4) sizeof(bodyBuffer), &indBody);
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *) &(bodyBufferLen), 
                              (sb4) sizeof(bodyBufferLen));
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (sms.routeId),
                              (sb4) sizeof(sms.routeId), &indRouteId);
    storageSelectStmt->define(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
                              (sb4) sizeof(sms.serviceId));
    storageSelectStmt->define(i++, SQLT_INT, (dvoid *)&(sms.priority),
                              (sb4) sizeof(sms.priority));
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
                              (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    storageSelectStmt->define(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
                              (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
    storageSelectStmt->define(i++, SQLT_UIN, (dvoid *)&(bodyTextLen), 
                              (sb4) sizeof(bodyTextLen));
    
    storageSelectStmt->bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
                            (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

const char* Archiver::storageDeleteSql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";
void Archiver::prepareStorageDeleteStmt() throw(StorageException)
{
    storageDeleteStmt = new Statement(storageConnection, 
                                      Archiver::storageDeleteSql, true);
    
    storageDeleteStmt->bind(1, SQLT_VNU, (dvoid *) &(smsId), 
                            (sb4) sizeof(smsId));
}

void Archiver::rebindBody() throw(StorageException)
{
    tripedBodyBufferLen = (bodyBufferLen<0 || 
                           bodyBufferLen>MAX_BODY_LENGTH || 
                           indBody != OCI_IND_NOTNULL) ? 0:bodyBufferLen;
    
    archiveInsertStmt->bind((CONST text *)"BODY", (sb4) 4*sizeof(char), 
                            SQLT_BIN, (dvoid *) bodyBuffer,
                            (sb4) tripedBodyBufferLen, &indBody);
}

const char* Archiver::archiveInsertSql = (const char*)
"INSERT INTO SMS_ARC (ID, ST, MR, OA, DA, DDA, SVC_TYPE,\
 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,\
 VALID_TIME, SUBMIT_TIME, ATTEMPTS, LAST_RESULT,\
 LAST_TRY_TIME, DR, BR, BODY, BODY_LEN, ROUTE_ID, SVC_ID, PRTY,\
 SRC_SME_ID, DST_SME_ID, TXT_LENGTH)\
 VALUES (:ID, :ST, :MR, :OA, :DA, :DDA, :SVC_TYPE,\
 :SRC_MSC, :SRC_IMSI, :SRC_SME_N, :DST_MSC, :DST_IMSI, :DST_SME_N,\
 :VALID_TIME, :SUBMIT_TIME, :ATTEMPTS, :LAST_RESULT,\
 :LAST_TRY_TIME, :DR, :BR, :BODY, :BODY_LEN,\
 :ROUTE_ID, :SVC_ID, :PRTY, :SRC_SME_ID, :DST_SME_ID, :TXT_LENGTH)";
void Archiver::prepareArchiveInsertStmt() throw(StorageException)
{
    archiveInsertStmt = new Statement(storageConnection, 
                                      Archiver::archiveInsertSql, true);
    
    ub4 i=1;
    archiveInsertStmt->bind(i++, SQLT_VNU, (dvoid *) &(smsId),
                            (sb4) sizeof(smsId));
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *) &(uState),
                            (sb4) sizeof(uState));
    archiveInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *) &(sms.messageReference), 
                            (sb4) sizeof(sms.messageReference));
    
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (svcType),
                            (sb4) sizeof(svcType), &indSvc);

    archiveInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.msc),
                            (sb4) sizeof(sms.originatingDescriptor.msc),
                            &indSrcMsc);
    archiveInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.imsi),
                            (sb4) sizeof(sms.originatingDescriptor.imsi),
                            &indSrcImsi);
    archiveInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *)&(sms.originatingDescriptor.sme),
                            (sb4) sizeof(sms.originatingDescriptor.sme),
                            &indSrcSme);
    archiveInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.msc),
                            (sb4) sizeof(sms.destinationDescriptor.msc),
                            &indDstMsc);
    archiveInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.imsi),
                            (sb4) sizeof(sms.destinationDescriptor.imsi),
                            &indDstImsi);
    archiveInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *)&(sms.destinationDescriptor.sme),
                            (sb4) sizeof(sms.destinationDescriptor.sme),
                            &indDstSme);
    archiveInsertStmt->bind(i++, SQLT_ODT, (dvoid *) &(validTime),
                            (sb4) sizeof(validTime));
    archiveInsertStmt->bind(i++, SQLT_ODT, (dvoid *) &(submitTime), 
                            (sb4) sizeof(submitTime));
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *)&(sms.attempts),
                            (sb4) sizeof(sms.attempts));
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *) &(sms.lastResult),
                            (sb4) sizeof(sms.lastResult));
    archiveInsertStmt->bind(i++, SQLT_ODT, (dvoid *) &(lastTime), 
                            (sb4) sizeof(lastTime), &indLastTime);
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *) &(sms.deliveryReport), 
                            (sb4) sizeof(sms.deliveryReport));
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *) &(sms.billingRecord), 
                            (sb4) sizeof(sms.billingRecord));

    archiveInsertStmt->bind(i++, SQLT_BIN, (dvoid *) bodyBuffer,
                            (sb4) sizeof(bodyBuffer), &indBody);
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *)&(bodyBufferLen),
                            (sb4) sizeof(bodyBufferLen));
    
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (sms.routeId),
                            (sb4) sizeof(sms.routeId), &indRouteId);
    archiveInsertStmt->bind(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
                            (sb4) sizeof(sms.serviceId));
    archiveInsertStmt->bind(i++, SQLT_INT, (dvoid *)&(sms.priority),
                            (sb4) sizeof(sms.priority));

    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
                            (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    archiveInsertStmt->bind(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
                            (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
    archiveInsertStmt->bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen),
                            (sb4) sizeof(bodyTextLen));
}

/*CREATE OR REPLACE PROCEDURE CREATE_BILLING_RECORD 
  (smsId IN RAW, 
   smsOaVal IN VARCHAR2, smsOaTon IN NUMBER, smsOaNpi IN NUMBER,
   smsOaMsc IN VARCHAR2, smsOaImsi IN VARCHAR2,
   smsDaVal IN VARCHAR2, smsDaTon IN NUMBER, smsDaNpi IN NUMBER,
   smsDaMsc IN VARCHAR2, smsDaImsi IN VARCHAR2,
   smsSubmit IN DATE, smsFinalized IN DATE, smsStatus IN NUMBER,
   smsRouteId IN VARCHAR2, smsServiceId IN NUMBER, smsTxtLen IN NUMBER) */

const char* Archiver::billingInsertSql = (const char*)
"BEGIN\
    CREATE_BILLING_RECORD\
    (:ID,\
     :OA_VAL, :OA_TON, :OA_NPI, :OA_MSC, :OA_IMSI,\
     :DA_VAL, :DA_TON, :DA_NPI, :DA_MSC, :DA_IMSI,\
     :SUBMIT, :FINALIZED, :LAST_RESULT,\
     :ROUTE_ID, :SERVICE_ID, :TXT_LENGTH);\
 END;";
void Archiver::prepareBillingInsertStmt() throw(StorageException)
{
    billingInsertStmt = new Statement(billingConnection, 
                                      Archiver::billingInsertSql, true);

    ub4 i = 1;
    billingInsertStmt->bind(i++, SQLT_VNU, (dvoid *) &(smsId),
                            (sb4) sizeof(smsId));
    
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.originatingAddress.value), 
                            (sb4) sizeof(sms.originatingAddress.value));
    billingInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.type), 
                            (sb4) sizeof(sms.originatingAddress.type));
    billingInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.plan), 
                            (sb4) sizeof(sms.originatingAddress.plan));
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.msc),
                            (sb4) sizeof(sms.originatingDescriptor.msc),
                            &indSrcMsc);
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.imsi),
                            (sb4) sizeof(sms.originatingDescriptor.imsi),
                            &indSrcImsi);
    
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.dealiasedDestinationAddress.value), 
                            (sb4) sizeof(sms.dealiasedDestinationAddress.value));
    billingInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *) &(sms.dealiasedDestinationAddress.type), 
                            (sb4) sizeof(sms.dealiasedDestinationAddress.type));
    billingInsertStmt->bind(i++, SQLT_UIN, 
                            (dvoid *) &(sms.dealiasedDestinationAddress.plan), 
                            (sb4) sizeof(sms.dealiasedDestinationAddress.plan));
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.msc),
                            (sb4) sizeof(sms.destinationDescriptor.msc),
                            &indDstMsc);
    billingInsertStmt->bind(i++, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.imsi),
                            (sb4) sizeof(sms.destinationDescriptor.imsi),
                            &indDstImsi);
    
    billingInsertStmt->bind(i++, SQLT_ODT, (dvoid *) &(submitTime), 
                            (sb4) sizeof(submitTime));
    billingInsertStmt->bind(i++, SQLT_ODT, (dvoid *) &(lastTime), 
                            (sb4) sizeof(lastTime), &indLastTime);
    
    billingInsertStmt->bind(i++, SQLT_UIN, (dvoid *) &(sms.lastResult),
                            (sb4) sizeof(sms.lastResult));
    
    billingInsertStmt->bind(i++, SQLT_STR, (dvoid *) (sms.routeId),
                            (sb4) sizeof(sms.routeId), &indRouteId);
    billingInsertStmt->bind(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
                            (sb4) sizeof(sms.serviceId));
    
    billingInsertStmt->bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen),
                            (sb4) sizeof(bodyTextLen));

}

/* --------------------- Archive Cleaner implementation -------------------- */ 

const unsigned SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_LIMIT   = 365; // days
const unsigned SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_DEFAULT = 30;  // days
void Archiver::Cleaner::loadCleanupAgeInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Archive.Cleaner.age");
        if (interval <= 0 || 
            interval > SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_LIMIT)
        {
            interval = SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_DEFAULT;
            log.warn("Cleanup age interval for archiver is incorrect "
                     "(should be between 1 and %u days) ! "
                     "Config parameter: <MessageStore.Archive.Cleaner.age> "
                     "Using default: %u", 
                     SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_LIMIT,
                     SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_DEFAULT;
        log.warn("Cleanup age interval for archiver missed "
                 "(it should be between 1 and %u days) ! "
                 "Config parameter: <MessageStore.Archive.Cleaner.age> "
                 "Using default: %u", 
                 SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_LIMIT,
                 SMSC_ARCHIVER_CLEANUP_AGE_INTERVAL_DEFAULT);
    }

    ageInterval = interval*3600*24; // in seconds
}

const unsigned SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_LIMIT = 3600; // seconds
const unsigned SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_DEFAULT =  5; // seconds
void Archiver::Cleaner::loadCleanupAwakeInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Archive.Cleaner.awake");
        if (interval <= 0 || 
            interval > SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_LIMIT)
        {
            interval = SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_DEFAULT;
            log.warn("Awake interval for archive cleaner is incorrect "
                     "(should be between 1 and %u seconds) ! "
                     "Config parameter: <MessageStore.Archive.Cleaner.awake> "
                     "Using default: %u", 
                     SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_LIMIT,
                     SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_DEFAULT;
        log.warn("Awake interval for archive cleaner missed "
                 "(it should be between 1 and %u seconds) ! "
                 "Config parameter: <MessageStore.Archive.Cleaner.awake> "
                 "Using default: %u", 
                 SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_LIMIT,
                 SMSC_ARCHIVER_CLEANUP_AWAKE_INTERVAL_DEFAULT);
    }

    awakeInterval = interval*1000; // in mseconds
}

const unsigned SMSC_ARCHIVER_CLEANUP_INTERVAL_LIMIT   = 3600; // seconds
const unsigned SMSC_ARCHIVER_CLEANUP_INTERVAL_DEFAULT = 60;   // seconds
void Archiver::Cleaner::loadCleanupInterval(Manager& config)
{
    int interval;
    try 
    {
        interval = config.getInt("MessageStore.Archive.Cleaner.interval");
        if (interval <= 0 || 
            interval > SMSC_ARCHIVER_CLEANUP_INTERVAL_LIMIT)
        {
            interval = SMSC_ARCHIVER_CLEANUP_INTERVAL_DEFAULT;
            log.warn("Cleanup interval for archiver is incorrect "
                     "(should be between 1 and %u seconds) ! "
                     "Config parameter: <MessageStore.Archive.Cleaner.interval> "
                     "Using default: %u", 
                     SMSC_ARCHIVER_CLEANUP_INTERVAL_LIMIT,
                     SMSC_ARCHIVER_CLEANUP_INTERVAL_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        interval = SMSC_ARCHIVER_CLEANUP_INTERVAL_DEFAULT;
        log.warn("Cleanup interval for archiver missed "
                 "(it should be between 1 and %u seconds) ! "
                 "Config parameter: <MessageStore.Archive.Cleaner.interval> "
                 "Using default: %u", 
                 SMSC_ARCHIVER_CLEANUP_INTERVAL_LIMIT,
                 SMSC_ARCHIVER_CLEANUP_INTERVAL_DEFAULT);
    }

    cleanupInterval = interval; // in seconds
}

Archiver::Cleaner::Cleaner(Manager& config, Connection* connection)
    throw(ConfigException) 
        : Thread(), log(Logger::getCategory("smsc.store.Archive.Cleaner")),
            bStarted(false), bNeedExit(false), cleanerConnection(connection)
{
    loadCleanupInterval(config);        // in seconds
    loadCleanupAgeInterval(config);     // in seconds
    loadCleanupAwakeInterval(config);   // in mseconds
    Start();
}
Archiver::Cleaner::~Cleaner()
{
    Stop();
}
void Archiver::Cleaner::Start()
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
void Archiver::Cleaner::Stop()
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

int Archiver::Cleaner::Execute()
{
    bool first = true;
    while (!bNeedExit)
    {
        awake.Wait((first) ? 1000:awakeInterval);
        if (bNeedExit) break;
        try 
        {
            //__trace__("Archive cleaning up ...");
            cleanup(); first = false;
            //__trace__("Archive cleaned up.");
        } 
        catch (StorageException& exc) 
        {
            awake.Wait(0); first = true;
            log.error("Exception occurred during archive cleanup : %s",
                      exc.what());
        }
    }
    exited.Signal();
    return 0;
}

void Archiver::Cleaner::cleanup() 
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
        if( log.isDebugEnabled() ) 
            log.debug("Archive cleanup: %d rows deleted (from %d to %d)",
                      cleanerDeleteStmt->getRowsAffectedCount(), 
                      oldDelete, toDelete);
        cleanerConnection->commit();
    }

    disconnect();
}

void Archiver::Cleaner::connect() 
    throw(StorageException)
{
    cleanerConnection->connect();
    prepareCleanerMinTimeStmt();
    prepareCleanerDeleteStmt();
}
void Archiver::Cleaner::disconnect() 
    throw(StorageException)
{
    cleanerConnection->disconnect();
}

const char* Archiver::Cleaner::cleanerMinTimeSql = (const char*)
"SELECT MIN(LAST_TRY_TIME) FROM SMS_ARC";
void Archiver::Cleaner::prepareCleanerMinTimeStmt() 
    throw(StorageException)
{
    cleanerMinTimeStmt = new Statement(cleanerConnection,
                                       Cleaner::cleanerMinTimeSql, true);

    cleanerMinTimeStmt->define(1, SQLT_ODT, (dvoid *) &(dbTime), 
                               (sb4) sizeof(dbTime), &indDbTime);
}

const char* Archiver::Cleaner::cleanerDeleteSql = (const char*)
"DELETE FROM SMS_ARC WHERE LAST_TRY_TIME<:LT";
void Archiver::Cleaner::prepareCleanerDeleteStmt() 
    throw(StorageException)
{
    cleanerDeleteStmt = new Statement(cleanerConnection, 
                                      Cleaner::cleanerDeleteSql, true);
    
}

}}


