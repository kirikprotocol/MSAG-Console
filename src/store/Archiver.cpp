
#include "Archiver.h"

#include <util/debug.h>

namespace smsc { namespace store 
{

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

const char* Archiver::selectSql = (const char*)
"SELECT ID, ST, MR, RM,\
 OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON, DA_NPI, DA_VAL,\
 VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME,\
 SRR, RD, ARC, PRI, PID, FCS, DCS, UDHI, UDL, UD FROM SMS_MSG\
 WHERE NOT ST=:ST";

const char* Archiver::insertSql = (const char*)
"INSERT INTO SMS_ARC VALUES (:ID, :ST, :MR, :RM,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :SRR, :RD, :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";

const char* Archiver::deleteSql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";

Archiver::Archiver(Manager& config)
    throw(ConfigException) 
        : log(Logger::getCategory("smsc.store.Archiver"))
{
    /*storageDBInstance = 
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

    __require__(storageDBInstance && storageDBUserName && storageDBUserPassword);
    __require__(archiveDBInstance && archiveDBUserName && archiveDBUserPassword);

    storageConnection = new Connection(
        storageDBInstance, storageDBUserName, storageDBUserPassword);
    
    archiveConnection = new Connection(
        archiveDBInstance, archiveDBUserName, archiveDBUserPassword);*/
}

Archiver::~Archiver() 
{
    if (storageDBInstance) delete storageDBInstance;
    if (storageDBUserName) delete storageDBUserName;
    if (storageDBUserPassword) delete storageDBUserPassword;
    if (archiveDBInstance) delete archiveDBInstance;
    if (archiveDBUserName) delete archiveDBUserName;
    if (archiveDBUserPassword) delete archiveDBUserPassword;

    if (selectStmt) delete selectStmt;
    if (deleteStmt) delete deleteStmt;
    if (insertStmt) delete insertStmt;
    
    delete storageConnection;
    delete archiveConnection;
}

void Archiver::archivate()
    throw(StorageException) 
{
    if (!storageConnection->isAvailable())
    {
        if (selectStmt) delete selectStmt;
        if (deleteStmt) delete deleteStmt;
        storageConnection->connect();
        selectStmt = new Statement(storageConnection, Archiver::selectSql);
        deleteStmt = new Statement(storageConnection, Archiver::deleteSql);
        // bind & define placeholders here !
    }
    
    if (!archiveConnection->isAvailable())
    {
        if (insertStmt) delete insertStmt;
        archiveConnection->connect();
        insertStmt = new Statement(archiveConnection, Archiver::insertSql);
        // bind & define placeholders here !
    }
    
    // do real job here
    unsigned uncommited = 0;
    try 
    {
        sword status;
        selectStmt->checkErr(selectStmt->execute());
        do
        {
            // Может тут вставить проверку не удалённых из SMS_MSG ?? 
            insertStmt->checkErr(insertStmt->execute());
            deleteStmt->checkErr(deleteStmt->execute());
            if (++uncommited == SMSC_ARCHIVER_TRANSACTION_COMMIT_INTERVAL)
            {
                archiveConnection->commit();
                storageConnection->commit();
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
    }
}

}}


