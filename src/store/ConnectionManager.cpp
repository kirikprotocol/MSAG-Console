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
const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_QUEUE_SIZE_LIMIT = 10000;

const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE = 10;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT = 1000;

const unsigned SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE = 5;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE_LIMIT = 1000;

void ConnectionPool::loadMaxSize(Manager& config)
{
    try 
    {
        size = (unsigned)config.getInt("MessageStore.Connections.max");
        if (!size || 
            size > SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT)
        {
            size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
            log.warn("Maximum ConnectionPool size is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.Connections.max> "
                     "Using default: %u",
                     SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT,
                     SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE);
        }
    } 
    catch (ConfigException& exc) 
    {
        size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
        log.warn("Maximum ConnectionPool size wasn't specified ! "
                 "Config parameter: <MessageStore.Connections.max> "
                 "Using default: %u",
                 SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE);
    }
}

void ConnectionPool::loadInitSize(Manager& config)
{
    try 
    {
        count = (unsigned)config.getInt("MessageStore.Connections.init");
        if (!count || 
            count > SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE_LIMIT)
        {
            count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
            log.warn("Init ConnectionPool size is incorrect "
                     "(should be between 0 and %u) ! "
                     "Config parameter: <MessageStore.Connections.init> "
                     "Using default: %u",
                     SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE_LIMIT, 
                     SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE);
        }
    } 
    catch (ConfigException& exc) 
    {
        count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
        log.warn("Init ConnectionPool size wasn't specified ! "
                 "Config parameter: <MessageStore.Connections.init> "
                 "Using default: %d",
                 SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE);
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
                  "Config parameter: <MessageStore.dbUserPassword>");
        throw;
    }
}

ConnectionPool::ConnectionPool(Manager& config)
    throw(ConfigException) 
        : queueLen(0), head(0L), tail(0L),
            idleCount(0), idleHead(0L), idleTail(0L),
                log(Logger::getCategory("smsc.store.ConnectionPool"))
{
    loadMaxSize(config);
    loadInitSize(config);
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
        connections.Push(connection);
        push(connection);
    }
}

void ConnectionPool::push(Connection* connection)
{
    __require__(connection);

    if (idleTail) 
    {
        idleTail->setNextConnection(connection);
    }
    idleTail = connection; 
    connection->setNextConnection(0L);
    if (!idleHead) idleHead = idleTail;
    idleCount++;
}

Connection* ConnectionPool::pop(void)
{
    Connection *connection = idleHead;
    if (idleHead) 
    {
        idleHead = idleHead->getNextConnection();
    }
    if (!idleHead) idleTail = 0L;

    if (connection) idleCount--;
    return connection;
}

ConnectionPool::~ConnectionPool()
{
    MutexGuard  guard(monitor);

    while (connections.Count())
    {
        Connection* connection=0L;
        (void) connections.Pop(connection);
        if (connection) delete connection;
    }
    
    delete dbInstance;
    delete dbUserName;
    delete dbUserPassword;
}

bool ConnectionPool::hasFreeConnections()
{
    MutexGuard  guard(monitor);
    return (idleCount || count < size);
}

Connection* ConnectionPool::getConnection()
{
    ConnectionQueue queue;
    MutexGuard  guard(monitor);
    
    if (head || (!idleCount && count >= size))
    {
        if (tail) tail->next = &queue;
        tail = &queue; queue.next = 0L;
        queueLen++;
        cond_init(&queue.condition,0,0);
        if (!head) head = tail;
        monitor.wait(&queue.condition);
        cond_destroy(&queue.condition);
        return queue.connection;
    }
    
    if (idleCount)
    {
        return pop();
    }
    else if (count < size) 
    {
        Connection* connection =
                    new Connection(dbInstance, dbUserName, dbUserPassword);
        (void) connections.Push(connection);
        count++;
        return connection; 
    }
}

void ConnectionPool::freeConnection(Connection* connection)
{
    __require__(connection);
    MutexGuard  guard(monitor);
    
    if (head && (count <= size))
    {
        ConnectionQueue *queue = head;
        head = head->next;
        if (!head) tail = 0L;
        queueLen--;
        queue->connection = connection;
        monitor.notify(&(queue->condition));
        return;
    }
    
    if (count > size)
    {
        for (int i=0; i<connections.Count(); i++)
        {
            if (connections[i] == connection)
            {
                connections.Delete(i); break;
            }
        }
        if (connection) delete connection;
        count--;
    }
    else 
    {
        push(connection);
    }
}

void ConnectionPool::setSize(unsigned new_size) 
{
    if (!new_size || new_size == size || count>size) return;
    MutexGuard  guard(monitor);
    
    if (new_size > SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT)
    {
        new_size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT;
        log.warn("Attempt to change ConnectionPool size "
                 "by more than allowed value "
                 "Using maximum possible : %u",
                 SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE_LIMIT);
    }
    
    if (new_size < size)
    {
        unsigned counter = size - new_size;
        while(idleCount && counter--)
        {
            Connection* connection = pop();
            for (int i=0; i<connections.Count(); i++)
            {
                if (connections[i] == connection)
                {
                    connections.Delete(i); break;
                }
            }
            if (connection) delete connection;
            count--;
        }
    } 
    else
    {
        unsigned counter = new_size - size;
        while (head && counter--)
        {
            Connection* connection = 
                    new Connection(dbInstance, dbUserName, dbUserPassword);
            (void) connections.Push(connection);
            count++;

            // Notify waiting threads & give them new connections
            ConnectionQueue *queue = head;
            head = head->next;
            if (!head) tail = 0L;
            queueLen--; 
            queue->connection = connection;
            monitor.notify(&(queue->condition));
        }
    }
    size = new_size;
}

/* ------------------------------- Connection -------------------------- */

Mutex Connection::connectLock;

Connection::Connection(const char* instance, 
                       const char* user, const char* password) 
    : isConnected(false), isDead(false), next(0L),
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
        MutexGuard  guard(connectLock);
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
            IsRejectStmt = new IsRejectedStatement(this);
        } 
        catch (StorageException& exc) 
        {
            if (IsRejectStmt) {
                delete IsRejectStmt; IsRejectStmt = 0L;
            }
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
    MutexGuard  guard(connectLock);
    
    if (isConnected)
    {
        if (IsRejectStmt) {
            delete IsRejectStmt; IsRejectStmt = 0L;
        }
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
    throw(StorageException)
{
    MutexGuard  guard(mutex);

    connect();

    SMSId   maxId;
    GetMaxIdStatement*  GetMaxIdStmt = new GetMaxIdStatement(this);
    checkErr(GetMaxIdStmt->execute(OCI_DEFAULT));
    GetMaxIdStmt->getSMSId(maxId);
    delete GetMaxIdStmt;
    return maxId;
}

void Connection::store(const SMS &sms, SMSId id) 
    throw(StorageException, DuplicateMessageException)
{
    MutexGuard  guard(mutex);

    connect();
    
    IsRejectStmt->setSMS(sms);
    try
    {
        checkErr(IsRejectStmt->execute(OCI_DEFAULT));
    }
    catch (StorageException& exc) 
    {
        throw exc;
    }
    if (IsRejectStmt->isRejected())
    {
        DuplicateMessageException   exc;
        //log.debug(exc.what());
        throw exc;
    }

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

    connect();

    RetriveStmt->setSMSId(id);
    sword status = RetriveStmt->execute(OCI_DEFAULT);
    if ((status) == OCI_NO_DATA)
    {
        NoSuchMessageException exc(id);
        //log.debug(exc.what());
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
        //log.debug(exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::replace(SMSId id, const SMS &sms) 
    throw(StorageException, NoSuchMessageException)
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
        //log.debug(exc.what());
        throw exc;
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::update(SMSId id, const State state, 
                        time_t operationTime, uint8_t fcs)
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);
    
    connect();
    
    StateUpdateStatement* UpdateStmt;

    switch (state)
    {
    case ENROUTE:
    case UNDELIVERABLE:
        UpdateStmt = new StateDateFcsUpdateStatement(this);
        ((StateDateFcsUpdateStatement *)UpdateStmt)->setOpTime(operationTime);
        ((StateDateFcsUpdateStatement *)UpdateStmt)->setFcs(fcs);
        break;
    
    case DELIVERED:
        UpdateStmt = new StateDateUpdateStatement(this);
        ((StateDateUpdateStatement *)UpdateStmt)->setOpTime(operationTime);
        break;

    case EXPIRED:
    case DELETED:
        UpdateStmt = new StateUpdateStatement(this);
        break;
    
    default:
        return; // throw exception ?
    }

    UpdateStmt->setState(state);
    UpdateStmt->setSMSId(id);
    try 
    {
        checkErr(UpdateStmt->execute(OCI_DEFAULT/*OCI_COMMIT_ON_SUCCESS*/));
    }
    catch (StorageException& exc) 
    {
        delete UpdateStmt;
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        throw exc;
    }

    if (!UpdateStmt->wasUpdated())
    {
        delete UpdateStmt;
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        NoSuchMessageException exc(id);
        //log.debug(exc.what());
        throw exc;
    }
    delete UpdateStmt;
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

}}

