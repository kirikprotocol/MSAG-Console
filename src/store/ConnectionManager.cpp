#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oci.h>
#include <orl.h>

#include "ConnectionManager.h"

#include <util/debug.h>
#include <sms/sms.h>

namespace smsc { namespace store 
{
using namespace smsc::sms;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

/* ----------------------------- ConnectionPool ------------------------ */

const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE = 10;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE = 5;

void ConnectionPool::loadMaxSize(Manager& config)
{
    try {
        size = (unsigned)config.getInt("MessageStore.Connections.max");
    } catch (ConfigException& exc) {
        size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
        log.warn("Max size wasn't specified ! "
                 "Using default: %d", size);
    }
}

void ConnectionPool::loadInitSize(Manager& config)
{
    try {
        count = (unsigned)config.getInt("MessageStore.Connections.init");
    } catch (ConfigException& exc) {
        count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
        log.warn("Init size wasn't specified ! "
                 "Using default: %d", count);
    }
}

void ConnectionPool::loadDBInstance(Manager& config)
    throw(ConfigException)
{
    try {
        dbInstance = config.getString("MessageStore.dbInstance");   
    } catch (ConfigException& exc) {
        log.error("DB instance name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserName(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserName = config.getString("MessageStore.dbUserName");   
    } catch (ConfigException& exc) {
        log.error("DB user name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserPassword(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserPassword = config.getString("MessageStore.dbUserPassword");   
    } catch (ConfigException& exc) {
        log.error("DB user password wasn't specified !");
        throw;
    }
}

ConnectionPool::ConnectionPool(Manager& config)
    throw(ConfigException, ConnectionFailedException) 
        : log(Logger::getCategory("smsc.store.ConnectionPool"))
{
    loadMaxSize(config);
    loadInitSize(config);
    loadDBInstance(config);
    loadDBUserName(config);
    loadDBUserPassword(config);

    if (size < count) 
    {
        size = count;
        log.warn("Specified size less than init size. "
                  "Using max: %d", size);
    }

    __require__(dbInstance && dbUserName && dbUserPassword);

    for (int i=0; i<count; i++)
    {
        Connection* connection = new Connection(this);
        (void) idle.Push(connection);
    }
}

ConnectionPool::~ConnectionPool()
{
    MutexGuard  guard(monitor);

    for (int i=0; i<idle.Count(); i++)
    {
        Connection* connection=0L;
        (void) idle.Pop(connection);
        if (connection) delete connection;
    }
    for (int i=0; i<busy.Count(); i++)
    {
        Connection* connection=0L;
        (void) busy.Pop(connection);
        if (connection) delete connection;
    }
    for (int i=0; i<dead.Count(); i++) 
    {
        Connection* connection=0L;
        (void) dead.Pop(connection);
        if (connection) delete connection;
    }
    
    delete dbInstance;
    delete dbUserName;
    delete dbUserPassword;
}

bool ConnectionPool::hasFreeConnections()
{
    return (count<size || idle.Count());
}

Connection* ConnectionPool::getConnection()
    throw(ConnectionFailedException)
{
    MutexGuard  guard(monitor);

    while (!hasFreeConnections()) monitor.wait();
    
    Connection* connection=0L;
    if (idle.Count())
    {
        (void) idle.Pop(connection);
        (void) busy.Push(connection);
    } 
    else if (count < size)
    {
        connection = new Connection(this);
        (void) busy.Push(connection);
        count++;
    }
    
    __require__(connection);
    return connection;
}

void ConnectionPool::freeConnection(Connection* connection)
{
    __require__(connection);
    MutexGuard  guard(monitor);
    
    Connection* tmp=0L;
    for (int i=0; i<busy.Count(); i++)
    {
        tmp = busy[i];
        if (tmp == connection) 
        {
            busy.Delete(i);
            if (count <= size)
            {
                (void) idle.Push(connection);
                monitor.notify();
            }
            else 
            {
                delete connection;
                count--;
            }
            return;
        }
    }
    for (int i=0; i<dead.Count(); i++)
    {
        tmp = dead[i];
        if (tmp == connection) 
        {
            dead.Delete(i);
            delete connection;
            count--;
            if (count < size) 
            {
                monitor.notify();
            }
            return;
        }
    }
}

void ConnectionPool::killConnection(Connection* connection)
{
    __require__(connection);
    MutexGuard  guard(monitor);

    Connection* tmp=0L;
    for (int i=0; i<busy.Count(); i++)
    {
        tmp = busy[i];
        if (tmp == connection) // set Connection dead
        { 
            busy.Delete(i); 
            (void) dead.Push(connection);
            break;
        }
    }
}

void ConnectionPool::setSize(unsigned new_size) 
{
    if (!new_size || new_size == size) return;
    MutexGuard  guard(monitor);
    
    if (new_size < size)
    {
        Connection* connection = 0L;
        for (int i=new_size-1; i<idle.Count(); i++)
        {
            (void) idle.Pop(connection);
            if (connection) delete connection;
            count--;
        }
    }

    if (count < (size = new_size))
    {
        monitor.notify();
    }
}

/* ----------------------------- ConnectionPool ------------------------ */

/* ------------------------------- Connection -------------------------- */

/*text* Connection::sqlUpdatePart = (text *)
"UPDATE SMS_MSG SET \
 ST=:ST, DELIVERY_TIME=:DELIVERY_TIME, FCS=:FCS WHERE ID=:ID";*/

Connection::Connection(ConnectionPool* pool) 
    throw(ConnectionFailedException) 
        : log(Logger::getCategory("smsc.store.Connection")), 
            owner(pool), envhp(0L), errhp(0L), svchp(0L), srvhp(0L), sesshp(0L) 
{
    __require__(owner);

    const char* dbName = owner->getDBInstance();
    const char* userName = owner->getDBUserName();
    const char* userPwd = owner->getDBUserPassword();
    
    __require__(userName && userPwd && dbName);

    // open connection to DB and begin user session 
    sword status;
    status = OCIEnvCreate(&envhp, OCI_OBJECT|OCI_ENV_NO_MUTEX, 
                          (dvoid *)0, 0, 0, 0, (size_t) 0, (dvoid **)0);
    __require__(status == OCI_SUCCESS);

    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, 
                            OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
    __require__(status == OCI_SUCCESS && errhp);

    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp,
                                OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)); 
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp,
                                OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0));
    checkConnErr(OCIServerAttach(srvhp, errhp, (text *)dbName,
                                 strlen(dbName), OCI_DEFAULT));
    checkConnErr(OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX,
                            (dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp));
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&sesshp,
                                OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0));   
    checkConnErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                            (dvoid *)userName, (ub4)strlen(userName),
                            (ub4) OCI_ATTR_USERNAME, errhp));
    checkConnErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                            (dvoid *)userPwd, (ub4) strlen(userPwd),
                            (ub4) OCI_ATTR_PASSWORD, errhp));
    checkConnErr(OCISessionBegin(svchp, errhp, sesshp, OCI_CRED_RDBMS,
                                 (ub4) OCI_DEFAULT));
    checkConnErr(OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
                            (dvoid *)sesshp, (ub4) 0,
                            (ub4) OCI_ATTR_SESSION, errhp));
    try
    {
        StoreStmt = new StoreStatement(this);
        RemoveStmt = new RemoveStatement(this);
        RetriveStmt = new RetriveStatement(this);
        ReplaceStmt = new ReplaceStatement(this);
    }
    catch (StorageException& exc) 
    {
        throw ConnectionFailedException(exc);
    }
}

Connection::~Connection()
{
    MutexGuard  guard(mutex);

    __require__(envhp && errhp && svchp);
    
    if (StoreStmt) delete StoreStmt;
    if (RemoveStmt) delete RemoveStmt;
    if (RetriveStmt) delete RetriveStmt;
    
    // logoff from database server
    (void) OCILogoff(svchp, errhp);

    // free envirounment handle (other handles will be freed too)
    (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
}

SMSId Connection::getMessagesCount()
    throw(ConnectionFailedException)
{
    MutexGuard  guard(mutex);

    SMSId   maxId;
    GetMaxIdStatement*  GetMaxIdStmt = new GetMaxIdStatement(this);
    checkConnErr(GetMaxIdStmt->execute(OCI_DEFAULT));
    maxId = GetMaxIdStmt->getMaxSMSId();
    delete GetMaxIdStmt;
    return maxId;
}

void Connection::store(const SMS &sms, SMSId id) 
    throw(StorageException)
{
    MutexGuard  guard(mutex);

    StoreStmt->setSMS(sms);
    StoreStmt->setSMSId(id);

    try 
    {
        checkErr(StoreStmt->execute(OCI_DEFAULT/*OCI_COMMIT_ON_SUCCESS*/));
        checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
    } 
    catch (StorageException& exc) 
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        throw exc;
    }
}

void Connection::retrive(SMSId id, SMS &sms) 
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);

    RetriveStmt->setSMSId(id);
    
    sword status = RetriveStmt->execute(OCI_DEFAULT);
    if ((status) == OCI_NO_DATA)
    {
        NoSuchMessageException exc(id);
        log.error("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    else 
    {
        checkErr(status);
        RetriveStmt->getSMS(sms);
    }
}

void Connection::remove(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);
    
    RemoveStmt->setSMSId(id);

    checkErr(RemoveStmt->execute(OCI_DEFAULT/*OCI_COMMIT_ON_SUCCESS*/));
    if (!RemoveStmt->wasRemoved()) 
    {
        NoSuchMessageException exc(id);
        log.error("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::replace(SMSId id, const SMS &sms) 
    throw(StorageException)
{
    MutexGuard  guard(mutex);
    
    ReplaceStmt->setSMS(sms);
    ReplaceStmt->setSMSId(id);

    try 
    {
        checkErr(ReplaceStmt->execute(OCI_DEFAULT/*OCI_COMMIT_ON_SUCCESS*/));
    }
    catch (StorageException& exc) 
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        throw exc;
    }

    if (!ReplaceStmt->wasReplaced())
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        NoSuchMessageException exc(id);
        log.error("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}


void Connection::checkConnErr(sword status) 
    throw(ConnectionFailedException)
{
    try 
    {
        checkErr(status);
    } 
    catch (StorageException& exc) 
    {
        throw ConnectionFailedException(exc);
    }
}

void Connection::checkErr(sword status) 
    throw(StorageException)
{
    text        errbuf[1024];
    ub4         buflen, errcode;
    
    switch (status)
    {
    case OCI_SUCCESS:
        return;

    case OCI_SUCCESS_WITH_INFO:
        //throw StoreException(status, "OCI_SUCCESS_WITH_INFO");
        return;
    
    case OCI_NO_DATA:
        strcpy((char *)errbuf, "OCI_NODATA");
        break;
        
    case OCI_NEED_DATA:
        strcpy((char *)errbuf, "OCI_NEED_DATA");
        break;
        
    case OCI_STILL_EXECUTING:
        strcpy((char *)errbuf, "OCI_STILL_EXECUTE");
        break;
    
    case OCI_INVALID_HANDLE:
        strcpy((char *)errbuf, "OCI_INVALID_HANDLE");
        break;
                
    case OCI_ERROR:
        (void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
                            (sb4 *)&errcode, errbuf,
                            (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
        status = errcode;
        break;  

    default:
        strcpy((char *)errbuf, "OCI_UNKNOWN_ERROR");
        break;
    }
    
    __require__(owner);
    owner->killConnection(this);

    StorageException exc((const char *)errbuf, (int)status);
    log.error("Storage Exception : %s\n", exc.what());
    throw exc;
}

/* ------------------------------- Connection -------------------------- */

}}

