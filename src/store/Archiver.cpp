
#include "Archiver.h"

#include <util/debug.h>

namespace smsc { namespace store 
{

using smsc::core::threads::Thread;
using namespace smsc::core::synchronization;

const uint8_t enrouteState = (uint8_t)ENROUTE;
const int SMSC_ARCHIVER_TRANSACTION_COMMIT_INTERVAL = 10;

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

const unsigned SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT = 100000;
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
            awakeInterval = SMSC_ARCHIVER_AWAKE_INTERVAL_DEFAULT;
            log.warn("Maximum timeout interval "
                     "(between Archiver activations) is incorrect "
                     "(should be between 1 and %u m-seconds) ! "
                     "Config parameter: <MessageStore.Archive.interval> "
                     "Using default: %u",
                     SMSC_ARCHIVER_AWAKE_INTERVAL_LIMIT,
                     SMSC_ARCHIVER_AWAKE_INTERVAL_DEFAULT);
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

const unsigned SMSC_ARCHIVER_MAX_FINALIZED_LIMIT = 10000;
const unsigned SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT = 1000;

void Archiver::loadMaxFinalizedCount(Manager& config)
{
    try 
    {
        maxFinalizedCount =
            (unsigned)config.getInt("MessageStore.Archive.interval");
        if (!maxFinalizedCount || 
            maxFinalizedCount > SMSC_ARCHIVER_MAX_FINALIZED_LIMIT)
        {
            maxFinalizedCount = SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT;
            log.warn("Maximum count of finalized messages "
                     "in storage (for Archiver activation) is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.Archive.count> "
                     "Using default: %u",
                     SMSC_ARCHIVER_MAX_FINALIZED_LIMIT,
                     SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT);
        }
    } 
    catch (ConfigException& exc) 
    {
        maxFinalizedCount = SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT;
        log.warn("Maximum count of finalized messages "
                 "in storage (for Archiver activation) wasn't specified ! "
                 "Config parameter: <MessageStore.Archive.count> "
                 "Using default: %u",
                 SMSC_ARCHIVER_MAX_FINALIZED_DEFAULT);
    }
}

Archiver::Archiver(Manager& config)
    throw(ConfigException, StorageException) 
        : Thread(), finalizedCount(0),
            log(Logger::getCategory("smsc.store.Archiver"))
{
    storageDBInstance = 
        loadDBInstance(config, "MessageStore.Storage.dbInstance");
    storageDBUserName = 
        loadDBUserName(config, "MessageStore.Storage.dbUserName");
    storageDBUserPassword = 
        loadDBUserPassword(config, "MessageStore.Storage.dbUserPassword");
    archiveDBInstance =
        loadDBInstance(config, "MessageStore.Archive.dbInstance");
    archiveDBUserName =
        loadDBUserName(config, "MessageStore.Archive.dbUserName");
    archiveDBUserPassword =
        loadDBUserPassword(config, "MessageStore.Archive.dbUserPassword");
    
    loadAwakeInterval(config);
    loadMaxFinalizedCount(config);

    __require__(storageDBInstance && storageDBUserName && storageDBUserPassword);
    __require__(archiveDBInstance && archiveDBUserName && archiveDBUserPassword);

    storageConnection = new Connection(
        storageDBInstance, storageDBUserName, storageDBUserPassword);
    
    archiveConnection = new Connection(
        archiveDBInstance, archiveDBUserName, archiveDBUserPassword);

    startup();
}


Archiver::~Archiver() 
{
    __trace__("Archiver destruction ...");
    exit.Signal();
    __trace__("Stop signal sent, waiting ...");
    exited.Wait();
    
    if (storageDBInstance) delete storageDBInstance;
    if (storageDBUserName) delete storageDBUserName;
    if (storageDBUserPassword) delete storageDBUserPassword;
    if (archiveDBInstance) delete archiveDBInstance;
    if (archiveDBUserName) delete archiveDBUserName;
    if (archiveDBUserPassword) delete archiveDBUserPassword;

    if (selectStmt) delete selectStmt;
    if (deleteStmt) delete deleteStmt;
    if (insertStmt) delete insertStmt;
    if (lookIdStmt) delete lookIdStmt;
    
    if (storageConnection) delete storageConnection;
    if (archiveConnection) delete archiveConnection;
    
    __trace__("Archiver destructed !");
}

const char* Archiver::countSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG WHERE NOT ST=:ST";

void Archiver::startup()
    throw(StorageException)
{
    connect();
    
    Statement countStmt(storageConnection, Archiver::countSql);
    countStmt.bind(1 , SQLT_UIN, (dvoid *) &(enrouteState),
                   (sb4) sizeof(enrouteState));
    countStmt.define(1 , SQLT_UIN, (dvoid *) &(finalizedCount), 
                     (sb4) sizeof(finalizedCount));
    countStmt.checkErr(countStmt.execute());
    
    archivate(true);
}

void Archiver::connect()
    throw(StorageException)
{
    if (!storageConnection->isAvailable())
    {
        if (selectStmt) delete selectStmt;
        if (deleteStmt) delete deleteStmt;
        storageConnection->connect();
        selectStmt = new Statement(storageConnection, Archiver::selectSql);
        prepareSelectStmt();
        deleteStmt = new Statement(storageConnection, Archiver::deleteSql);
        prepareDeleteStmt();
    }
    
    if (!archiveConnection->isAvailable())
    {
        if (insertStmt) delete insertStmt;
        archiveConnection->connect();
        insertStmt = new Statement(archiveConnection, Archiver::insertSql);
        prepareInsertStmt();
        lookIdStmt = new Statement(archiveConnection, Archiver::lookIdSql);
        prepareLookIdStmt();
    }
}

const char* Archiver::storageMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), '0000000000000000') FROM SMS_MSG";
const char* Archiver::archiveMaxIdSql = (const char*)
"SELECT NVL(MAX(ID), '0000000000000000') FROM SMS_ARC";
SMSId Archiver::getMaxId() 
    throw(StorageException)
{
    connect();
    SMSId storageId, archiveId;

    GetIdStatement storageMaxIdStmt(storageConnection,
                                    Archiver::storageMaxIdSql);
    GetIdStatement archiveMaxIdStmt(archiveConnection, 
                                    Archiver::archiveMaxIdSql);

    storageMaxIdStmt.checkErr(storageMaxIdStmt.execute());
    storageMaxIdStmt.getSMSId(storageId);
    archiveMaxIdStmt.checkErr(archiveMaxIdStmt.execute());
    archiveMaxIdStmt.getSMSId(archiveId);
    
    return ((storageId > archiveId) ? storageId : archiveId);
}

void Archiver::incrementFinalizedCount(unsigned count)
{
    MutexGuard  guard(finalizedMutex);  

    if ((finalizedCount += count) >= maxFinalizedCount) 
    {
        if (!job.isSignaled()) 
        {
            job.Signal();
            __trace__("Signal sent !");
        }
    }
}
void Archiver::decrementFinalizedCount(unsigned count)
{
    MutexGuard  guard(finalizedMutex);
    
    if ((finalizedCount -= count) < 0)
    {
        finalizedCount = 0;
    }
}

int Archiver::Execute()
{
    __trace__("Archiver started !");
    do 
    {
        job.Wait(awakeInterval);
        try 
        {
            __trace__("Doing archivation job ...");
            archivate(false);
            __trace__("Archivation job done !");
        } 
        catch (StorageException& exc) 
        {
            log.error("Exception occurred during archivation process : %s",
                      exc.what());
        }
    } 
    while (!exit.isSignaled());
    
    exited.Signal();
    __trace__("Archiver exited !");
    return 0;
}

void Archiver::archivate(bool first)
    throw(StorageException) 
{
    connect();

    // do real job here
    unsigned uncommited = 0;
    try 
    {
        sword status = selectStmt->execute();
        if (status == OCI_NO_DATA) return;
        selectStmt->checkErr(status);
        do
        {
            if (bNeedArchivate)
            {
                if (first)
                {
                    lookIdStmt->checkErr(lookIdStmt->execute());
                    if (idCounter == 0)
                    {
                        insertStmt->checkErr(insertStmt->execute());    
                    }
                } 
                else 
                {
                    insertStmt->checkErr(insertStmt->execute());
                }
            }
            deleteStmt->checkErr(deleteStmt->execute());
            if (++uncommited == SMSC_ARCHIVER_TRANSACTION_COMMIT_INTERVAL)
            {
                archiveConnection->commit();
                storageConnection->commit();
                decrementFinalizedCount(uncommited);
                uncommited = 0;
            }
        } 
        while ((status = selectStmt->fetch()) == OCI_SUCCESS ||
               status == OCI_SUCCESS_WITH_INFO);

        if (status != OCI_NO_DATA)
            selectStmt->checkErr(status);
    }
    catch (StorageException& exc)
    {
        archiveConnection->rollback();
        storageConnection->rollback();
        throw exc;
    }

    if (uncommited)
    {
        archiveConnection->commit();
        storageConnection->commit();
        decrementFinalizedCount(uncommited);
    }
}

const char* Archiver::selectSql = (const char*)
"SELECT ID, ST, MR, RM,\
 OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON, DA_NPI, DA_VAL,\
 VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME,\
 SRR, RD, ARC, PRI, PID, FCS, DCS, UDHI, UDL, UD FROM SMS_MSG\
 WHERE NOT ST=:ST";
void Archiver::prepareSelectStmt() throw(StorageException)
{
    selectStmt->define(1, SQLT_BIN, (dvoid *) &(id),
                       (sb4) sizeof(id));
    selectStmt->define(2 , SQLT_UIN, (dvoid *) &(uState), 
                       (sb4) sizeof(uState));
    selectStmt->define(3 , SQLT_UIN, (dvoid *) &(msgReference),
                       (sb4) sizeof(msgReference));
    selectStmt->define(4 , SQLT_UIN, (dvoid *) &(msgIdentifier),
                       (sb4) sizeof(msgIdentifier));
    selectStmt->define(5 , SQLT_UIN, (dvoid *) &(oaLenght),
                       (sb4) sizeof(oaLenght));
    selectStmt->define(6 , SQLT_UIN, (dvoid *) &(oaType),
                       (sb4) sizeof(oaType));
    selectStmt->define(7 , SQLT_UIN, (dvoid *) &(oaPlan),
                       (sb4) sizeof(oaPlan));
    selectStmt->define(8 , SQLT_STR, (dvoid *) (oaValue),
                       (sb4) sizeof(oaValue));
    selectStmt->define(9 , SQLT_UIN, (dvoid *)&(daLenght),
                       (sb4) sizeof(daLenght));
    selectStmt->define(10, SQLT_UIN, (dvoid *) &(daType),
                       (sb4) sizeof(daType));
    selectStmt->define(11, SQLT_UIN, (dvoid *) &(daPlan),
                       (sb4) sizeof(daPlan));
    selectStmt->define(12, SQLT_STR, (dvoid *) (daValue),
                       (sb4) sizeof(daValue));
    selectStmt->define(13, SQLT_ODT, (dvoid *) &(validTime),
                       (sb4) sizeof(validTime));
    selectStmt->define(14, SQLT_ODT, (dvoid *) &(waitTime),
                       (sb4) sizeof(waitTime));
    selectStmt->define(15, SQLT_ODT, (dvoid *) &(submitTime),
                       (sb4) sizeof(submitTime));
    selectStmt->define(16, SQLT_ODT, (dvoid *) &(deliveryTime),
                       (sb4) sizeof(deliveryTime));
    selectStmt->define(17, SQLT_AFC, (dvoid *) &(bStatusReport),
                       (sb4) sizeof(bStatusReport));
    selectStmt->define(18, SQLT_AFC, (dvoid *) &(bRejectDuplicates),
                       (sb4) sizeof(bRejectDuplicates));
    selectStmt->define(19, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
                       (sb4) sizeof(bNeedArchivate));
    selectStmt->define(20, SQLT_UIN, (dvoid *) &(priority),
                       (sb4) sizeof(priority));
    selectStmt->define(21, SQLT_UIN, (dvoid *) &(protocolIdentifier),
                       (sb4) sizeof(protocolIdentifier));
    selectStmt->define(22, SQLT_UIN, (dvoid *) &(failureCause),
                       (sb4) sizeof(failureCause));
    selectStmt->define(23, SQLT_UIN, (dvoid *) &(dataScheme),
                       (sb4) sizeof(dataScheme));
    selectStmt->define(24, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                       (sb4) sizeof(bHeaderIndicator));
    selectStmt->define(25, SQLT_UIN, (dvoid *) &(dataLenght),
                       (sb4) sizeof(dataLenght));
    selectStmt->define(26, SQLT_BIN, (dvoid *) (data),
                       (sb4) sizeof(data));
    
    selectStmt->bind(1 , SQLT_UIN, (dvoid *) &(enrouteState),
                     (sb4) sizeof(enrouteState));
}

const char* Archiver::deleteSql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";
void Archiver::prepareDeleteStmt() throw(StorageException)
{
    deleteStmt->bind(1, SQLT_BIN, (dvoid *) &(id), (sb4) sizeof(id));
}

const char* Archiver::lookIdSql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_ARC WHERE ID=:ID";
void Archiver::prepareLookIdStmt() throw(StorageException)
{
    lookIdStmt->define(1 , SQLT_UIN, (dvoid *) &(idCounter), 
                       (sb4) sizeof(idCounter));
    lookIdStmt->bind(1, SQLT_BIN, (dvoid *) &(id), (sb4) sizeof(id));
}

const char* Archiver::insertSql = (const char*)
"INSERT INTO SMS_ARC VALUES (:ID, :ST, :MR, :RM,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :SRR, :RD, :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";
void Archiver::prepareInsertStmt() throw(StorageException)
{
    insertStmt->bind(1, SQLT_BIN, (dvoid *) &(id),
                     (sb4) sizeof(id));
    insertStmt->bind(2 , SQLT_UIN, (dvoid *) &(uState), 
                     (sb4) sizeof(uState));
    insertStmt->bind(3 , SQLT_UIN, (dvoid *) &(msgReference),
                     (sb4) sizeof(msgReference));
    insertStmt->bind(4 , SQLT_UIN, (dvoid *) &(msgIdentifier),
                     (sb4) sizeof(msgIdentifier));
    insertStmt->bind(5 , SQLT_UIN, (dvoid *) &(oaLenght),
                     (sb4) sizeof(oaLenght));
    insertStmt->bind(6 , SQLT_UIN, (dvoid *) &(oaType),
                     (sb4) sizeof(oaType));
    insertStmt->bind(7 , SQLT_UIN, (dvoid *) &(oaPlan),
                     (sb4) sizeof(oaPlan));
    insertStmt->bind(8 , SQLT_STR, (dvoid *) (oaValue),
                     (sb4) sizeof(oaValue));
    insertStmt->bind(9 , SQLT_UIN, (dvoid *)&(daLenght),
                     (sb4) sizeof(daLenght));
    insertStmt->bind(10, SQLT_UIN, (dvoid *) &(daType),
                     (sb4) sizeof(daType));
    insertStmt->bind(11, SQLT_UIN, (dvoid *) &(daPlan),
                     (sb4) sizeof(daPlan));
    insertStmt->bind(12, SQLT_STR, (dvoid *) (daValue),
                     (sb4) sizeof(daValue));
    insertStmt->bind(13, SQLT_ODT, (dvoid *) &(validTime),
                     (sb4) sizeof(validTime));
    insertStmt->bind(14, SQLT_ODT, (dvoid *) &(waitTime),
                     (sb4) sizeof(waitTime));
    insertStmt->bind(15, SQLT_ODT, (dvoid *) &(submitTime),
                     (sb4) sizeof(submitTime));
    insertStmt->bind(16, SQLT_ODT, (dvoid *) &(deliveryTime),
                     (sb4) sizeof(deliveryTime));
    insertStmt->bind(17, SQLT_AFC, (dvoid *) &(bStatusReport),
                     (sb4) sizeof(bStatusReport));
    insertStmt->bind(18, SQLT_AFC, (dvoid *) &(bRejectDuplicates),
                     (sb4) sizeof(bRejectDuplicates));
    insertStmt->bind(19, SQLT_UIN, (dvoid *) &(priority),
                     (sb4) sizeof(priority));
    insertStmt->bind(20, SQLT_UIN, (dvoid *) &(protocolIdentifier),
                     (sb4) sizeof(protocolIdentifier));
    insertStmt->bind(21, SQLT_UIN, (dvoid *) &(failureCause),
                     (sb4) sizeof(failureCause));
    insertStmt->bind(22, SQLT_UIN, (dvoid *) &(dataScheme),
                     (sb4) sizeof(dataScheme));
    insertStmt->bind(23, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                     (sb4) sizeof(bHeaderIndicator));
    insertStmt->bind(24, SQLT_UIN, (dvoid *) &(dataLenght),
                     (sb4) sizeof(dataLenght));
    insertStmt->bind(25, SQLT_BIN, (dvoid *) (data),
                     (sb4) sizeof(data));
}

}}


