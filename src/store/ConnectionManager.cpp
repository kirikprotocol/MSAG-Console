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

const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_QUEUE_SIZE = 200;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE = 10;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE = 5;

void ConnectionPool::loadMaxSize(Manager& config)
{
    try 
    {
        size = (unsigned)config.getInt("MessageStore.Connections.max");
    } 
    catch (ConfigException& exc) 
    {
        size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
        log.warn("Maximum ConnectionPool size wasn't specified ! "
                 "Config parameter: <MessageStore.Connections.max> "
                 "Using default: %d", size);
    }
}

void ConnectionPool::loadInitSize(Manager& config)
{
    try 
    {
        count = (unsigned)config.getInt("MessageStore.Connections.init");
    } 
    catch (ConfigException& exc) 
    {
        count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
        log.warn("Init ConnectionPool size wasn't specified ! "
                 "Config parameter: <MessageStore.Connections.init> "
                 "Using default: %d", count);
    }
}

void ConnectionPool::loadMaxQueueSize(Manager& config)
{
    try 
    {
        maxQueueSize = 
            (unsigned)config.getInt("MessageStore.Connections.queue");
    } 
    catch (ConfigException& exc) 
    {
        maxQueueSize = SMSC_DEFAULT_CONNECTION_POOL_MAX_QUEUE_SIZE;
        log.warn("Maximum count of pending requests to ConnectionPool "
                 "for connections wasn't specified ! "
                 "Config parameter: <MessageStore.Connections.queue> "
                 "Using default: %d", maxQueueSize);
    }
}

void ConnectionPool::loadDBInstance(Manager& config)
    throw(ConfigException)
{
    try 
    {
        dbInstance = config.getString("MessageStore.dbInstance");   
    } 
    catch (ConfigException& exc) 
    {
        log.error("DB instance name wasn't specified ! "
                  "Config parameter: <MessageStore.dbInstance>");
        throw;
    }
}

void ConnectionPool::loadDBUserName(Manager& config)
    throw(ConfigException)
{
    try 
    {
        dbUserName = config.getString("MessageStore.dbUserName");   
    } 
    catch (ConfigException& exc) 
    {
        log.error("DB user name wasn't specified ! "
                  "Config parameter: <MessageStore.dbUserName>");
        throw;
    }
}

void ConnectionPool::loadDBUserPassword(Manager& config)
    throw(ConfigException)
{
    try 
    {
        dbUserPassword = config.getString("MessageStore.dbUserPassword");   
    } 
    catch (ConfigException& exc) 
    {
        log.error("DB user password wasn't specified ! "
                  "Config parameter: <MessageStore.UserPassword>");
        throw;
    }
}

ConnectionPool::ConnectionPool(Manager& config)
    throw(ConfigException) 
        : queueLen(0), log(Logger::getCategory("smsc.store.ConnectionPool"))
{
    loadMaxSize(config);
    loadInitSize(config);
    loadMaxQueueSize(config);
    loadDBInstance(config);
    loadDBUserName(config);
    loadDBUserPassword(config);
    
    if (size < count) 
    {
        size = count;
        log.warn("Specified size of ConnectionPool less than it's init size. "
                 "Using maximum value defined:  %d", size);
    }

    __require__(dbInstance && dbUserName && dbUserPassword);

    for (int i=0; i<count; i++)
    {
        Connection* connection = 
                    new Connection(dbInstance, dbUserName, dbUserPassword);
        idle.Push(connection);
    }
    head = tail = 0L; // Reset ConnectionQueue
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
    
    delete dbInstance;
    delete dbUserName;
    delete dbUserPassword;
}

bool ConnectionPool::hasFreeConnections()
{
    MutexGuard  guard(monitor);
    return (count<size || idle.Count());
}

Connection* ConnectionPool::getConnection()
    throw(TooLargeQueueException)
{
    ConnectionQueue queue;
    MutexGuard  guard(monitor);
    
    if (head || (!idle.Count() && count>=size))
    {
        if (queueLen >= maxQueueSize)
        {
            TooLargeQueueException exc;
            log.error(exc.what());
            throw exc;
        }
        if (tail) tail->next = &queue;
        tail = &queue; queue.next = 0L;
        queueLen++;
        cond_init(&queue.condition,0,0);
        if (!head) head = tail;
        monitor.wait(&queue.condition);
        cond_destroy(&queue.condition);
        return queue.connection;
    }
    
    Connection* connection=0L;
    if (idle.Count())
    {
        (void) idle.Pop(connection);
        (void) busy.Push(connection);
    } 
    else if (count < size)
    {
        connection = new Connection(dbInstance, dbUserName, dbUserPassword);
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
    
    if (head)
    {
        ConnectionQueue *queue = head;
        head = head->next;
        if (!head) tail = 0L;
        queueLen--;
        queue->connection = connection;
        monitor.notify(&(queue->condition));
        return;
    }
    
    for (int i=0; i<busy.Count(); i++)
    {
        if (busy[i] == connection) 
        {
            busy.Delete(i);
            if (count <= size)
            {
                (void) idle.Push(connection);
            }
            else 
            {
                delete connection;
                count--;
            }
            return;
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
    else
    {
        unsigned counter = new_size - size;
        while (counter--)
        {
            Connection* connection = 
                    new Connection(dbInstance, dbUserName, dbUserPassword);
            if (head)
            {   // Notify waiting threads & give them new connections
                ConnectionQueue *queue = head;
                head = head->next;
                if (!head) tail = 0L;
                queueLen--;
                queue->connection = connection;
                monitor.notify(&(queue->condition));
            }
            else 
            {
                (void) idle.Push(connection);
            }
        }
    }
    size = new_size;
}

/* ------------------------------- Connection -------------------------- */

Connection::Connection(const char* instance, 
                       const char* user, const char* password) 
    : isConnected(false), isDead(false),
        dbInstance(instance), dbUserName(user), dbUserPassword(password), 
            StoreStmt(0L), RemoveStmt(0L), RetriveStmt(0L), ReplaceStmt(0L), 
                envhp(0L), errhp(0L), svchp(0L), srvhp(0L), sesshp(0L), 
                    log(Logger::getCategory("smsc.store.Connection"))
{
    __require__(dbInstance && dbUserName && dbUserPassword);
}
        
Connection::~Connection() 
{
    MutexGuard  guard(mutex);
    disconnect();
};

void Connection::connect() 
    throw(ConnectionFailedException) 
{
    if (isDead && isConnected) disconnect();
    
    if (!isConnected)
    {
        try
        {
            // open connection to DB and begin user session 
            checkErr(OCIEnvCreate(&envhp, OCI_OBJECT|OCI_ENV_NO_MUTEX,
                                  (dvoid *)0, 0, 0, 0, (size_t) 0, (dvoid **)0));
            checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp,
                                    OCI_HTYPE_ERROR, (size_t) 0, (dvoid **)0));
            checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp,
                                    OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)); 
            checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp,
                                    OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0));
            checkErr(OCIServerAttach(srvhp, errhp, (text *)dbInstance,
                                     strlen(dbInstance), OCI_DEFAULT));
            checkErr(OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX,
                                (dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp));
            checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&sesshp,
                                    OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0));   
            checkErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                                (dvoid *)dbUserName, (ub4)strlen(dbUserName),
                                (ub4) OCI_ATTR_USERNAME, errhp));
            checkErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                                (dvoid *)dbUserPassword, 
                                (ub4) strlen(dbUserPassword),
                                (ub4) OCI_ATTR_PASSWORD, errhp));
            checkErr(OCISessionBegin(svchp, errhp, sesshp, OCI_CRED_RDBMS,
                                     (ub4) OCI_DEFAULT));
            checkErr(OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
                                (dvoid *)sesshp, (ub4) 0,
                                (ub4) OCI_ATTR_SESSION, errhp));
            
            StoreStmt = new StoreStatement(this);
            RemoveStmt = new RemoveStatement(this);
            RetriveStmt = new RetriveStatement(this);
            ReplaceStmt = new ReplaceStatement(this);
        } 
        catch (StorageException& exc) 
        {
            if (ReplaceStmt) {
                delete ReplaceStmt; ReplaceStmt = 0L;
            }
            if (RetriveStmt) {
                delete RetriveStmt; RetriveStmt = 0L;
            }
            if (RemoveStmt) {
                delete RemoveStmt; RemoveStmt = 0L;
            }
            if (StoreStmt) {
                delete StoreStmt; StoreStmt = 0L;
            }
            
            if (errhp && svchp) {
            // logoff from database server
                (void) OCILogoff(svchp, errhp);
            }
            if (envhp) {
            // free envirounment handle, all derrived handles will be freed too
                (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
            }
            
            throw ConnectionFailedException(exc);
        }
        
        isConnected = true; isDead = false;
    }
}

void Connection::disconnect() 
{
    if (isConnected)
    {
        if (ReplaceStmt) {
            delete ReplaceStmt; ReplaceStmt = 0L;
        }
        if (RetriveStmt) {
            delete RetriveStmt; RetriveStmt = 0L;
        }
        if (RemoveStmt) {
            delete RemoveStmt; RemoveStmt = 0L;
        }
        if (StoreStmt) {
            delete StoreStmt; StoreStmt = 0L;
        }

        if (errhp && svchp) {
        // logoff from database server
            (void) OCILogoff(svchp, errhp);
        }
        if (envhp) {
        // free envirounment handle, all derrived handles will be freed too
            (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
        }

        isConnected = false; isDead = false;
    }
}

SMSId Connection::getMessagesCount()
    throw(ConnectionFailedException, StorageException)
{
    MutexGuard  guard(mutex);

    connect();

    SMSId   maxId;
    GetMaxIdStatement*  GetMaxIdStmt = new GetMaxIdStatement(this);
    checkErr(GetMaxIdStmt->execute(OCI_DEFAULT));
    maxId = GetMaxIdStmt->getMaxSMSId();
    delete GetMaxIdStmt;
    return maxId;
}

void Connection::store(const SMS &sms, SMSId id) 
    throw(ConnectionFailedException, StorageException)
{
    MutexGuard  guard(mutex);

    connect();

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
    throw(ConnectionFailedException, StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);

    connect();

    RetriveStmt->setSMSId(id);
    sword status = RetriveStmt->execute(OCI_DEFAULT);
    if ((status) == OCI_NO_DATA)
    {
        NoSuchMessageException exc(id);
        log.warn("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    else 
    {
        checkErr(status);
        RetriveStmt->getSMS(sms);
    }
}

void Connection::remove(SMSId id) 
    throw(ConnectionFailedException, StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);

    connect();
    
    RemoveStmt->setSMSId(id);
    try 
    {
        checkErr(RemoveStmt->execute(OCI_DEFAULT/*OCI_COMMIT_ON_SUCCESS*/));
    }
    catch (StorageException& exc) 
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        throw exc;
    }
    
    if (!RemoveStmt->wasRemoved()) 
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        NoSuchMessageException exc(id);
        log.warn("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::replace(SMSId id, const SMS &sms) 
    throw(ConnectionFailedException, StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);
    
    connect();
    
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
        log.warn("Storage Exception : %s\n", exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::checkErr(sword status) 
    throw(StorageException)
{
    text        errbuf[1024];
    ub4         buflen, errcode = status;
    
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
        if (errhp)
        {
            (void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
                                (sb4 *)&errcode, errbuf,
                                (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
        }
        else 
        {
            strcpy((char *)errbuf, "NO_ERROR_DESCRIPTION");
        }
        break;  

    default:
        strcpy((char *)errbuf, "OCI_UNKNOWN_ERROR");
        break;
    }
    
    isDead = true;

    StorageException exc((const char *)errbuf, (int)errcode, (int)status);
    log.error("Storage Exception : %s\n", exc.what());
    throw exc;
}

/*void Connection::checkConnErr(sword status) 
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
}*/

/* ------------------------------- Connection -------------------------- */

}}

