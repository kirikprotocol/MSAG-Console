
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

    if (storageSelectStmt) delete storageSelectStmt;
    if (storageDeleteStmt) delete storageDeleteStmt;
    if (archiveInsertStmt) delete archiveInsertStmt;
    if (billingInsertStmt) delete billingInsertStmt;
    
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
        billingLookIdStmt->checkErr(billingLookIdStmt->execute());
    }
    
    if (idCounter == 0)
    {
        billingPutIdStmt->checkErr(billingPutIdStmt->execute());
        // TO DO : Isert more code here ! Actual conversion to SMS_BR
        billingInsertStmt->checkErr(billingInsertStmt->execute());
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
            billingCleanIdsStmt->checkErr(billingCleanIdsStmt->execute());
            billingConnection->commit();
        }
        sword status = storageSelectStmt->execute();
        if (status == OCI_NO_DATA) return;
        storageSelectStmt->checkErr(status);
        do
        {
            billing(first);
            
            if (bNeedArchivate == 'Y')
            {
                archiveInsertStmt->checkErr(archiveInsertStmt->execute());
            } 
            else log.debug("Message wasn't archived (needArchivate != 'Y')!");
            
            storageDeleteStmt->checkErr(storageDeleteStmt->execute());

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
            storageSelectStmt->checkErr(status);
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
    maxIdStmt.checkErr(maxIdStmt.execute());
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
    if (billingPutIdStmt) delete billingPutIdStmt;
    billingPutIdStmt = new Statement(billingConnection,
                                     Archiver::billingPutIdSql);

    billingPutIdStmt->bind(1 , SQLT_BIN, (dvoid *) &(id),
                           (sb4) sizeof(id));
}

const char* Archiver::billingLookIdSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_IDS WHERE ID=:ID";
void Archiver::prepareBillingLookIdStmt()
    throw(StorageException)
{
    if (billingLookIdStmt) delete billingLookIdStmt;
    billingLookIdStmt = new Statement(billingConnection,
                                     Archiver::billingLookIdSql);

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
    if (billingCleanIdsStmt) delete billingCleanIdsStmt;
    billingCleanIdsStmt = new Statement(billingConnection,
                                        Archiver::billingCleanIdsSql);
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
    countStmt.checkErr(countStmt.execute());
}

const char* Archiver::storageSelectSql = (const char*)
"SELECT ID, ST, MR,\
 OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON, DA_NPI, DA_VAL,\
 VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME,\
 ARC, PRI, PID, FCS, DCS, UDHI, UDL, UD FROM SMS_MSG\
 WHERE NOT ST=:ENROUTE ORDER BY DELIVERY_TIME ASC";
void Archiver::prepareStorageSelectStmt() throw(StorageException)
{
    if (storageSelectStmt) delete storageSelectStmt;
    storageSelectStmt = new Statement(storageConnection, 
                                      Archiver::storageSelectSql);
    
    storageSelectStmt->define(1, SQLT_BIN, (dvoid *) &(id),
                              (sb4) sizeof(id));
    storageSelectStmt->define(2 , SQLT_UIN, (dvoid *) &(uState), 
                              (sb4) sizeof(uState));
    storageSelectStmt->define(3 , SQLT_UIN, (dvoid *) &(msgReference),
                              (sb4) sizeof(msgReference));
    storageSelectStmt->define(4 , SQLT_UIN, (dvoid *) &(oaLenght),
                              (sb4) sizeof(oaLenght));
    storageSelectStmt->define(5 , SQLT_UIN, (dvoid *) &(oaType),
                              (sb4) sizeof(oaType));
    storageSelectStmt->define(6 , SQLT_UIN, (dvoid *) &(oaPlan),
                              (sb4) sizeof(oaPlan));
    storageSelectStmt->define(7 , SQLT_STR, (dvoid *) (oaValue),
                              (sb4) sizeof(oaValue));
    storageSelectStmt->define(8 , SQLT_UIN, (dvoid *)&(daLenght),
                              (sb4) sizeof(daLenght));
    storageSelectStmt->define(9 , SQLT_UIN, (dvoid *) &(daType),
                              (sb4) sizeof(daType));
    storageSelectStmt->define(10, SQLT_UIN, (dvoid *) &(daPlan),
                              (sb4) sizeof(daPlan));
    storageSelectStmt->define(11, SQLT_STR, (dvoid *) (daValue),
                              (sb4) sizeof(daValue));
    storageSelectStmt->define(12, SQLT_ODT, (dvoid *) &(validTime),
                              (sb4) sizeof(validTime));
    storageSelectStmt->define(13, SQLT_ODT, (dvoid *) &(waitTime),
                              (sb4) sizeof(waitTime));
    storageSelectStmt->define(14, SQLT_ODT, (dvoid *) &(submitTime),
                              (sb4) sizeof(submitTime));
    storageSelectStmt->define(15, SQLT_ODT, (dvoid *) &(deliveryTime),
                              (sb4) sizeof(deliveryTime));
    storageSelectStmt->define(16, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
                              (sb4) sizeof(bNeedArchivate));
    storageSelectStmt->define(17, SQLT_UIN, (dvoid *) &(priority),
                              (sb4) sizeof(priority));
    storageSelectStmt->define(18, SQLT_UIN, (dvoid *) &(protocolIdentifier),
                              (sb4) sizeof(protocolIdentifier));
    storageSelectStmt->define(19, SQLT_UIN, (dvoid *) &(failureCause),
                              (sb4) sizeof(failureCause));
    storageSelectStmt->define(20, SQLT_UIN, (dvoid *) &(dataScheme),
                              (sb4) sizeof(dataScheme));
    storageSelectStmt->define(21, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                              (sb4) sizeof(bHeaderIndicator));
    storageSelectStmt->define(22, SQLT_UIN, (dvoid *) &(dataLenght),
                              (sb4) sizeof(dataLenght));
    storageSelectStmt->define(23, SQLT_BIN, (dvoid *) (data),
                              (sb4) sizeof(data));
    
    storageSelectStmt->bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
                            (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

const char* Archiver::storageDeleteSql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";
void Archiver::prepareStorageDeleteStmt() throw(StorageException)
{
    if (storageDeleteStmt) delete storageDeleteStmt;
    storageDeleteStmt = new Statement(storageConnection, 
                                      Archiver::storageDeleteSql);
    
    storageDeleteStmt->bind(1, SQLT_BIN, (dvoid *) &(id), 
                            (sb4) sizeof(id));
}

const char* Archiver::archiveInsertSql = (const char*)
"INSERT INTO SMS_ARC VALUES (:ID, :ST, :MR,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";
void Archiver::prepareArchiveInsertStmt() throw(StorageException)
{
    if (archiveInsertStmt) delete archiveInsertStmt;
    archiveInsertStmt = new Statement(storageConnection, 
                                      Archiver::archiveInsertSql);
    
    archiveInsertStmt->bind(1, SQLT_BIN, (dvoid *) &(id),
                            (sb4) sizeof(id));
    archiveInsertStmt->bind(2 , SQLT_UIN, (dvoid *) &(uState), 
                            (sb4) sizeof(uState));
    archiveInsertStmt->bind(3 , SQLT_UIN, (dvoid *) &(msgReference),
                            (sb4) sizeof(msgReference));
    archiveInsertStmt->bind(4 , SQLT_UIN, (dvoid *) &(oaLenght),
                            (sb4) sizeof(oaLenght));
    archiveInsertStmt->bind(5 , SQLT_UIN, (dvoid *) &(oaType),
                            (sb4) sizeof(oaType));
    archiveInsertStmt->bind(6 , SQLT_UIN, (dvoid *) &(oaPlan),
                            (sb4) sizeof(oaPlan));
    archiveInsertStmt->bind(7 , SQLT_STR, (dvoid *) (oaValue),
                            (sb4) sizeof(oaValue));
    archiveInsertStmt->bind(8 , SQLT_UIN, (dvoid *)&(daLenght),
                            (sb4) sizeof(daLenght));
    archiveInsertStmt->bind(9 , SQLT_UIN, (dvoid *) &(daType),
                            (sb4) sizeof(daType));
    archiveInsertStmt->bind(10, SQLT_UIN, (dvoid *) &(daPlan),
                            (sb4) sizeof(daPlan));
    archiveInsertStmt->bind(11, SQLT_STR, (dvoid *) (daValue),
                            (sb4) sizeof(daValue));
    archiveInsertStmt->bind(12, SQLT_ODT, (dvoid *) &(validTime),
                            (sb4) sizeof(validTime));
    archiveInsertStmt->bind(13, SQLT_ODT, (dvoid *) &(waitTime),
                            (sb4) sizeof(waitTime));
    archiveInsertStmt->bind(14, SQLT_ODT, (dvoid *) &(submitTime),
                            (sb4) sizeof(submitTime));
    archiveInsertStmt->bind(15, SQLT_ODT, (dvoid *) &(deliveryTime),
                            (sb4) sizeof(deliveryTime));
    archiveInsertStmt->bind(16, SQLT_UIN, (dvoid *) &(priority),
                            (sb4) sizeof(priority));
    archiveInsertStmt->bind(17, SQLT_UIN, (dvoid *) &(protocolIdentifier),
                            (sb4) sizeof(protocolIdentifier));
    archiveInsertStmt->bind(18, SQLT_UIN, (dvoid *) &(failureCause),
                            (sb4) sizeof(failureCause));
    archiveInsertStmt->bind(19, SQLT_UIN, (dvoid *) &(dataScheme),
                            (sb4) sizeof(dataScheme));
    archiveInsertStmt->bind(20, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                            (sb4) sizeof(bHeaderIndicator));
    archiveInsertStmt->bind(21, SQLT_UIN, (dvoid *) &(dataLenght),
                            (sb4) sizeof(dataLenght));
    archiveInsertStmt->bind(22, SQLT_BIN, (dvoid *) (data),
                            (sb4) sizeof(data));
}

const char* Archiver::billingInsertSql = (const char*)
"INSERT INTO SMS_BR VALUES (:ID, :ST, :MR,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";
void Archiver::prepareBillingInsertStmt() throw(StorageException)
{
    if (billingInsertStmt) delete billingInsertStmt;
    billingInsertStmt = new Statement(billingConnection, 
                                      Archiver::billingInsertSql);
    
    billingInsertStmt->bind(1, SQLT_BIN, (dvoid *) &(id),
                            (sb4) sizeof(id));
    billingInsertStmt->bind(2 , SQLT_UIN, (dvoid *) &(uState), 
                            (sb4) sizeof(uState));
    billingInsertStmt->bind(3 , SQLT_UIN, (dvoid *) &(msgReference),
                            (sb4) sizeof(msgReference));
    billingInsertStmt->bind(4 , SQLT_UIN, (dvoid *) &(oaLenght),
                            (sb4) sizeof(oaLenght));
    billingInsertStmt->bind(5 , SQLT_UIN, (dvoid *) &(oaType),
                            (sb4) sizeof(oaType));
    billingInsertStmt->bind(6 , SQLT_UIN, (dvoid *) &(oaPlan),
                            (sb4) sizeof(oaPlan));
    billingInsertStmt->bind(7 , SQLT_STR, (dvoid *) (oaValue),
                            (sb4) sizeof(oaValue));
    billingInsertStmt->bind(8 , SQLT_UIN, (dvoid *)&(daLenght),
                            (sb4) sizeof(daLenght));
    billingInsertStmt->bind(9 , SQLT_UIN, (dvoid *) &(daType),
                            (sb4) sizeof(daType));
    billingInsertStmt->bind(10, SQLT_UIN, (dvoid *) &(daPlan),
                            (sb4) sizeof(daPlan));
    billingInsertStmt->bind(11, SQLT_STR, (dvoid *) (daValue),
                            (sb4) sizeof(daValue));
    billingInsertStmt->bind(12, SQLT_ODT, (dvoid *) &(validTime),
                            (sb4) sizeof(validTime));
    billingInsertStmt->bind(13, SQLT_ODT, (dvoid *) &(waitTime),
                            (sb4) sizeof(waitTime));
    billingInsertStmt->bind(14, SQLT_ODT, (dvoid *) &(submitTime),
                            (sb4) sizeof(submitTime));
    billingInsertStmt->bind(15, SQLT_ODT, (dvoid *) &(deliveryTime),
                            (sb4) sizeof(deliveryTime));
    billingInsertStmt->bind(16, SQLT_UIN, (dvoid *) &(priority),
                            (sb4) sizeof(priority));
    billingInsertStmt->bind(17, SQLT_UIN, (dvoid *) &(protocolIdentifier),
                            (sb4) sizeof(protocolIdentifier));
    billingInsertStmt->bind(18, SQLT_UIN, (dvoid *) &(failureCause),
                            (sb4) sizeof(failureCause));
    billingInsertStmt->bind(19, SQLT_UIN, (dvoid *) &(dataScheme),
                            (sb4) sizeof(dataScheme));
    billingInsertStmt->bind(20, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                            (sb4) sizeof(bHeaderIndicator));
    billingInsertStmt->bind(21, SQLT_UIN, (dvoid *) &(dataLenght),
                            (sb4) sizeof(dataLenght));
    billingInsertStmt->bind(22, SQLT_BIN, (dvoid *) (data),
                            (sb4) sizeof(data));
}

}}


