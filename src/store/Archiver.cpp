
#include "Archiver.h"

#include <util/debug.h>

namespace smsc { namespace store 
{

using smsc::core::threads::Thread;
using namespace smsc::core::synchronization;

const char* Archiver::loadDBInstance(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        return config.getString(cat);   
    } 
    catch (ConfigException& exc) 
    {
        log.error("DB instance name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
const char* Archiver::loadDBUserName(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        return config.getString(cat);   
    } 
    catch (ConfigException& exc) 
    {
        log.error("DB user name wasn't specified ! "
                  "Config parameter: <%s>", cat);
        throw;
    }
}
const char* Archiver::loadDBUserPassword(Manager& config, const char* cat)
    throw(ConfigException)
{
    try 
    {
        return config.getString(cat);   
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

Archiver::Archiver(Manager& config) throw(ConfigException, StorageException) 
    : Thread(), finalizedCount(0), bStarted(false),
        storageDBInstance(0L), storageDBUserName(0L), storageDBUserPassword(0L),
        billingDBInstance(0L), billingDBUserName(0L), billingDBUserPassword(0L),
            storageSelectStmt(0L), storageDeleteStmt(0L), archiveInsertStmt(0L),
            billingInsertStmt(0L), billingCleanIdsStmt(0L), billingLookIdStmt(0L),
            billingPutIdStmt(0L), log(Logger::getCategory("smsc.store.Archiver"))
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
}


Archiver::~Archiver() 
{
    __trace__("Archiver destruction ...");
    
    this->Stop();
    
    if (storageDBInstance) delete storageDBInstance;
    if (storageDBUserName) delete storageDBUserName;
    if (storageDBUserPassword) delete storageDBUserPassword;
    if (billingDBInstance) delete billingDBInstance;
    if (billingDBUserName) delete billingDBUserName;
    if (billingDBUserPassword) delete billingDBUserPassword;

    if (storageConnection) delete storageConnection;
    if (billingConnection) delete billingConnection;
    
    __trace__("Archiver destructed !");
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
    
    if (bStarted)
    {
        exit.Signal();
        job.Signal();
        exited.Wait();
        bStarted = false;
    }
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

    if ((finalizedCount += count) >= maxFinalizedCount) 
    {
        if (bStarted && !job.isSignaled()) 
        {
            job.Signal();
            __trace2__("Signal sent, Finalized count is : %d !\n", 
                       finalizedCount);
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
    __trace__("Archiver started !");
    bool first = true;
    do 
    {
        job.Wait((first) ? 0:awakeInterval);
        if (exit.isSignaled()) break;
        try 
        {
            __trace__("Doing archivation job ...\n");
            job.Signal();
            archivate(first); first = false; 
            job.Wait(0);
            __trace__("Archivation job done !\n");
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
    __trace__("Archiver exited !");
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
    
    if (idCounter == 0)
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
            billing(first);
            
            if (bNeedArchivate == 'Y')
            {
                archiveInsertStmt->check(archiveInsertStmt->execute());
            } 
            else log.debug("Message wasn't archived (needArchivate != 'Y')!");
            
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
    }
    catch (StorageException& exc)
    {
        billingConnection->rollback();
        storageConnection->rollback();
        throw exc;
    }

    if (uncommited)
    {
        billingConnection->commit();
        storageConnection->commit();
        decrementFinalizedCount(uncommited);
    }
}

const char* Archiver::billingMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), '0000000000000000') FROM SMS_IDS";
const char* Archiver::storageMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), '0000000000000000') FROM SMS_MSG";
const char* Archiver::archiveMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), '0000000000000000') FROM SMS_ARC";
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
    SMSId billingId = getMaxUsedId(billingConnection, 
                                   Archiver::billingMaxIdSql);
    
    storageId = (storageId > archiveId) ? storageId : archiveId;
    return ((storageId > billingId) ? storageId : billingId);
}

const char* Archiver::billingPutIdSql = (const char*)
"INSERT INTO SMS_IDS VALUES (:ID)";
void Archiver::prepareBillingPutIdStmt()
    throw(StorageException)
{
    billingPutIdStmt = new Statement(billingConnection,
                                     Archiver::billingPutIdSql, true);
    
    billingPutIdStmt->bind(1 , SQLT_BIN, (dvoid *) &(id),
                           (sb4) sizeof(id));
}

const char* Archiver::billingLookIdSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_IDS WHERE ID=:ID";
void Archiver::prepareBillingLookIdStmt()
    throw(StorageException)
{
    billingLookIdStmt = new Statement(billingConnection,
                                     Archiver::billingLookIdSql, true);
    
    billingLookIdStmt->bind(1 , SQLT_BIN, (dvoid *) &(id),
                            (sb4) sizeof(id));
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
"SELECT ID, ST, MR,\
 OA_TON, OA_NPI, OA_VAL, SRC_MSC, SRC_IMSI, SRC_SME_N,\
 DA_TON, DA_NPI, DA_VAL, DST_MSC, DST_IMSI, DST_SME_N,\
 WAIT_TIME, VALID_TIME, SUBMIT_TIME,\
 ATTEMPTS, LAST_RESULT, LAST_TRY_TIME,\
 DR, ARC, PRI, PID, UDHI, DCS, UDL, UD FROM SMS_MSG\
 WHERE NOT ST=:ENROUTE ORDER BY LAST_TRY_TIME ASC";
void Archiver::prepareStorageSelectStmt() throw(StorageException)
{
    storageSelectStmt = new Statement(storageConnection, 
                                      Archiver::storageSelectSql, true);
    
    storageSelectStmt->define(1, SQLT_BIN, (dvoid *) &(id),
                              (sb4) sizeof(id));
    storageSelectStmt->define(2 , SQLT_UIN, (dvoid *) &(uState), 
                              (sb4) sizeof(uState));
    storageSelectStmt->define(3 , SQLT_UIN, 
                              (dvoid *) &(sms.messageReference),
                              (sb4) sizeof(sms.messageReference));
    storageSelectStmt->define(4 , SQLT_UIN, 
                              (dvoid *) &(sms.originatingAddress.type),
                              (sb4) sizeof(sms.originatingAddress.type));
    storageSelectStmt->define(5 , SQLT_UIN, 
                              (dvoid *) &(sms.originatingAddress.plan),
                              (sb4) sizeof(sms.originatingAddress.plan));
    storageSelectStmt->define(6 , SQLT_STR, 
                              (dvoid *) (sms.originatingAddress.value),
                              (sb4) sizeof(sms.originatingAddress.value));
    storageSelectStmt->define(7 , SQLT_STR, 
                              (dvoid *) (sms.originatingDescriptor.msc),
                              (sb4) sizeof(sms.originatingDescriptor.msc));
    storageSelectStmt->define(8 , SQLT_STR, 
                              (dvoid *) (sms.originatingDescriptor.imsi),
                              (sb4) sizeof(sms.originatingDescriptor.imsi));
    storageSelectStmt->define(9 , SQLT_UIN, 
                              (dvoid *)&(sms.originatingDescriptor.sme),
                              (sb4) sizeof(sms.originatingDescriptor.sme));
    storageSelectStmt->define(10, SQLT_UIN, 
                              (dvoid *) &(sms.destinationAddress.type),
                              (sb4) sizeof(sms.destinationAddress.type));
    storageSelectStmt->define(11, SQLT_UIN, 
                              (dvoid *) &(sms.destinationAddress.plan),
                              (sb4) sizeof(sms.destinationAddress.plan));
    storageSelectStmt->define(12, SQLT_STR, 
                              (dvoid *) (sms.destinationAddress.value),
                              (sb4) sizeof(sms.destinationAddress.value));
    storageSelectStmt->define(13, SQLT_STR, 
                              (dvoid *) (sms.destinationDescriptor.msc),
                              (sb4) sizeof(sms.destinationDescriptor.msc));
    storageSelectStmt->define(14, SQLT_STR, 
                              (dvoid *) (sms.destinationDescriptor.imsi),
                              (sb4) sizeof(sms.destinationDescriptor.imsi));
    storageSelectStmt->define(15 , SQLT_UIN, 
                              (dvoid *)&(sms.destinationDescriptor.sme),
                              (sb4) sizeof(sms.destinationDescriptor.sme));
    storageSelectStmt->define(16, SQLT_ODT, (dvoid *) &(waitTime),
                              (sb4) sizeof(waitTime));
    storageSelectStmt->define(17, SQLT_ODT, (dvoid *) &(validTime),
                              (sb4) sizeof(validTime));
    storageSelectStmt->define(18, SQLT_ODT, (dvoid *) &(submitTime),
                              (sb4) sizeof(submitTime));
    storageSelectStmt->define(19, SQLT_UIN, (dvoid *)&(sms.attempts),
                              (sb4) sizeof(sms.attempts));
    storageSelectStmt->define(20, SQLT_UIN, (dvoid *) &(sms.failureCause),
                              (sb4) sizeof(sms.failureCause));
    storageSelectStmt->define(21, SQLT_ODT, (dvoid *) &(lastTime),
                              (sb4) sizeof(lastTime));
    storageSelectStmt->define(22, SQLT_UIN, (dvoid *) &(sms.deliveryReport),
                              (sb4) sizeof(sms.deliveryReport));
    storageSelectStmt->define(23, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
                              (sb4) sizeof(bNeedArchivate));
    storageSelectStmt->define(24, SQLT_UIN, (dvoid *) &(sms.priority),
                              (sb4) sizeof(sms.priority));
    storageSelectStmt->define(25, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier),
                              (sb4) sizeof(sms.protocolIdentifier));
    storageSelectStmt->define(26, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                              (sb4) sizeof(bHeaderIndicator));
    storageSelectStmt->define(27, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
                              (sb4) sizeof(sms.messageBody.scheme));
    storageSelectStmt->define(28, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
                              (sb4) sizeof(sms.messageBody.lenght));
    storageSelectStmt->define(29, SQLT_BIN, (dvoid *) (sms.messageBody.data),
                              (sb4) sizeof(sms.messageBody.data));
    
    storageSelectStmt->bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
                            (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

const char* Archiver::storageDeleteSql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";
void Archiver::prepareStorageDeleteStmt() throw(StorageException)
{
    storageDeleteStmt = new Statement(storageConnection, 
                                      Archiver::storageDeleteSql, true);
    
    storageDeleteStmt->bind(1, SQLT_BIN, (dvoid *) &(id), 
                            (sb4) sizeof(id));
}

const char* Archiver::archiveInsertSql = (const char*)
"INSERT INTO SMS_ARC (ID, ST, MR,\
 OA_TON, OA_NPI, OA_VAL, SRC_MSC, SRC_IMSI, SRC_SME_N,\
 DA_TON, DA_NPI, DA_VAL, DST_MSC, DST_IMSI, DST_SME_N,\
 WAIT_TIME, VALID_TIME, SUBMIT_TIME, ATTEMPTS, LAST_RESULT,\
 LAST_TRY_TIME, RD, ARC, PRI, PID, UDHI, DCS, UDL, UD)\
 VALUES (:ID, :ST, :MR,\
 :OA_TON, :OA_NPI, :OA_VAL, :SRC_MSC, :SRC_IMSI, :SRC_SME_N,\
 :DA_TON, :DA_NPI, :DA_VAL, :DST_MSC, :DST_IMSI, :DST_SME_N,\
 :WAIT_TIME, :VALID_TIME, :SUBMIT_TIME, :ATTEMPTS, :LAST_RESULT,\
 :LAST_TRY_TIME, :RD, :ARC, :PRI, :PID, :UDHI, :DCS, :UDL, :UD)";
void Archiver::prepareArchiveInsertStmt() throw(StorageException)
{
    archiveInsertStmt = new Statement(storageConnection, 
                                      Archiver::archiveInsertSql, true);
    
    archiveInsertStmt->bind(1 , SQLT_BIN, (dvoid *) &(id),
                            (sb4) sizeof(id));
    archiveInsertStmt->bind(2 , SQLT_UIN, (dvoid *) &(uState),
                            (sb4) sizeof(uState));
    archiveInsertStmt->bind(3 , SQLT_UIN, 
                            (dvoid *) &(sms.messageReference), 
                            (sb4) sizeof(sms.messageReference));
    archiveInsertStmt->bind(4 , SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.type), 
                            (sb4) sizeof(sms.originatingAddress.type));
    archiveInsertStmt->bind(5 , SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.plan), 
                            (sb4) sizeof(sms.originatingAddress.plan));
    archiveInsertStmt->bind(6 , SQLT_STR, 
                            (dvoid *) (sms.originatingAddress.value), 
                            (sb4) sizeof(sms.originatingAddress.value));
    archiveInsertStmt->bind(7 , SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.msc),
                            (sb4) sizeof(sms.originatingDescriptor.msc));
    archiveInsertStmt->bind(8 , SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.imsi),
                            (sb4) sizeof(sms.originatingDescriptor.imsi));
    archiveInsertStmt->bind(9 , SQLT_UIN, 
                            (dvoid *)&(sms.originatingDescriptor.sme),
                            (sb4) sizeof(sms.originatingDescriptor.sme));
    archiveInsertStmt->bind(10, SQLT_UIN, 
                            (dvoid *) &(sms.destinationAddress.type), 
                            (sb4) sizeof(sms.destinationAddress.type));
    archiveInsertStmt->bind(11, SQLT_UIN, 
                            (dvoid *) &(sms.destinationAddress.plan), 
                            (sb4) sizeof(sms.destinationAddress.plan));
    archiveInsertStmt->bind(12, SQLT_STR, 
                            (dvoid *) (sms.destinationAddress.value), 
                            (sb4) sizeof(sms.destinationAddress.value));
    archiveInsertStmt->bind(13, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.msc),
                            (sb4) sizeof(sms.destinationDescriptor.msc));
    archiveInsertStmt->bind(14, SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.imsi),
                            (sb4) sizeof(sms.destinationDescriptor.imsi));
    archiveInsertStmt->bind(15, SQLT_UIN, 
                            (dvoid *)&(sms.destinationDescriptor.sme),
                            (sb4) sizeof(sms.destinationDescriptor.sme));
    archiveInsertStmt->bind(16, SQLT_ODT, (dvoid *) &(waitTime), 
                            (sb4) sizeof(waitTime));
    archiveInsertStmt->bind(17, SQLT_ODT, (dvoid *) &(validTime),
                            (sb4) sizeof(validTime));
    archiveInsertStmt->bind(18, SQLT_ODT, (dvoid *) &(submitTime), 
                            (sb4) sizeof(submitTime));
    archiveInsertStmt->bind(19, SQLT_UIN, (dvoid *)&(sms.attempts),
                            (sb4) sizeof(sms.attempts));
    archiveInsertStmt->bind(20, SQLT_UIN, (dvoid *) &(sms.failureCause),
                            (sb4) sizeof(sms.failureCause));
    archiveInsertStmt->bind(21, SQLT_ODT, (dvoid *) &(lastTime), 
                            (sb4) sizeof(lastTime));
    archiveInsertStmt->bind(22, SQLT_UIN, (dvoid *) &(sms.deliveryReport), 
                            (sb4) sizeof(sms.deliveryReport));
    archiveInsertStmt->bind(23, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
                            (sb4) sizeof(bNeedArchivate));
    archiveInsertStmt->bind(24, SQLT_UIN, (dvoid *) &(sms.priority), 
                            (sb4) sizeof(sms.priority));
    archiveInsertStmt->bind(25, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier), 
                            (sb4) sizeof(sms.protocolIdentifier));
    archiveInsertStmt->bind(26, SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
                            (sb4) sizeof(bHeaderIndicator));
    archiveInsertStmt->bind(27, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
                            (sb4) sizeof(sms.messageBody.scheme));
    archiveInsertStmt->bind(28, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
                            (sb4) sizeof(sms.messageBody.lenght));
    archiveInsertStmt->bind(29, SQLT_BIN, (dvoid *) (sms.messageBody.data), 
                            (sb4) sizeof(sms.messageBody.data));
}

const char* Archiver::billingInsertSql = (const char*)
"INSERT INTO SMS_BR (ATTEMPT, CI, DATA,\
 DST_ADDR, DST_IMSI, DST_MSC, DST_NPI, DST_SME_N, DST_TON,\
 FD, LAST_RESULT, N, PRTY, RP, SMS_UPLOAD_N,\
 SRC_ADDR, SRC_IMSI, SRC_MSC, SRC_NPI, SRC_SME_N, SRC_TON,\
 STATUS, SUBMIT_FD, TD, TXT_LENGTH)\
 VALUES (:1,0,NULL,:2,:3,:4,:5,:6,:7,\
 0,:8,0,:9,:10,0,\
 :11,:12,:13,:14,:15,:16,\
 :17,:18,0,:19)";

void Archiver::prepareBillingInsertStmt() throw(StorageException)
{
    billingInsertStmt = new Statement(billingConnection, 
                                      Archiver::billingInsertSql, true);
    
    billingInsertStmt->bind(1 , SQLT_UIN, (dvoid *)&(sms.attempts),
                            (sb4) sizeof(sms.attempts));
    billingInsertStmt->bind(2 , SQLT_STR, 
                            (dvoid *) (sms.destinationAddress.value), 
                            (sb4) sizeof(sms.destinationAddress.value));
    billingInsertStmt->bind(3 , SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.imsi),
                            (sb4) sizeof(sms.destinationDescriptor.imsi));
    billingInsertStmt->bind(4 , SQLT_STR, 
                            (dvoid *) (sms.destinationDescriptor.msc),
                            (sb4) sizeof(sms.destinationDescriptor.msc));
    billingInsertStmt->bind(5 , SQLT_UIN, 
                            (dvoid *) &(sms.destinationAddress.plan), 
                            (sb4) sizeof(sms.destinationAddress.plan));
    billingInsertStmt->bind(6 , SQLT_UIN, 
                            (dvoid *)&(sms.destinationDescriptor.sme),
                            (sb4) sizeof(sms.destinationDescriptor.sme));
    billingInsertStmt->bind(7 , SQLT_UIN, 
                            (dvoid *) &(sms.destinationAddress.type), 
                            (sb4) sizeof(sms.destinationAddress.type));
    billingInsertStmt->bind(8 , SQLT_UIN, (dvoid *) &(sms.failureCause),
                            (sb4) sizeof(sms.failureCause));
    billingInsertStmt->bind(9 , SQLT_UIN, (dvoid *) &(sms.priority), 
                            (sb4) sizeof(sms.priority));
    billingInsertStmt->bind(10, SQLT_AFC, (dvoid *) &(bStatusReport), 
                            (sb4) sizeof(bStatusReport));
    billingInsertStmt->bind(11, SQLT_STR, 
                            (dvoid *) (sms.originatingAddress.value), 
                            (sb4) sizeof(sms.originatingAddress.value));
    billingInsertStmt->bind(12, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.imsi),
                            (sb4) sizeof(sms.originatingDescriptor.imsi));
    billingInsertStmt->bind(13, SQLT_STR, 
                            (dvoid *) (sms.originatingDescriptor.msc),
                            (sb4) sizeof(sms.originatingDescriptor.msc));
    billingInsertStmt->bind(14, SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.plan), 
                            (sb4) sizeof(sms.originatingAddress.plan));
    billingInsertStmt->bind(15, SQLT_UIN, 
                            (dvoid *)&(sms.originatingDescriptor.sme),
                            (sb4) sizeof(sms.originatingDescriptor.sme));
    billingInsertStmt->bind(16, SQLT_UIN, 
                            (dvoid *) &(sms.originatingAddress.type), 
                            (sb4) sizeof(sms.originatingAddress.type));
    billingInsertStmt->bind(17, SQLT_UIN, (dvoid *) &(uState),
                            (sb4) sizeof(uState));
    billingInsertStmt->bind(18, SQLT_ODT, (dvoid *) &(lastTime), 
                            (sb4) sizeof(lastTime));
    billingInsertStmt->bind(19, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
                            (sb4) sizeof(sms.messageBody.lenght));
}

}}


