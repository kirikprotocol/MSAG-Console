
#include "DataSource.h"

namespace smsc { namespace db
{
using smsc::logger::Logger;

/* --------- Abstract Connection Management (ConnectionPool) --------------- */

Hash<DataSourceFactory *>*  DataSourceFactory::registry = 0;
Mutex                       DataSourceFactory::registryLock;

const unsigned SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE = 10;
const unsigned SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE_LIMIT = 1000;

bool Connection::_registerStatement(const char* id, Statement* statement)
{
    if (!statement || !id || id[0] == '\0' || statementsRegistry.Exists(id)) return false;
    statementsRegistry.Insert(id, statement);
    return true;
}
bool Connection::registerStatement(const char* id, Statement* statement)
{
    MutexGuard guard(statementsRegistryLock);
    return _registerStatement(id, statement);
}
bool Connection::unregisterStatement(const char* id)
{
    if (!id || id[0] == '\0') return false;
    MutexGuard guard(statementsRegistryLock);
    Statement** statementPtr = statementsRegistry.GetPtr(id);
    if (!statementPtr) return false;
    if (*statementPtr) delete *statementPtr;
    statementsRegistry.Delete(id);
    return true;
}
Statement* Connection::_getStatement(const char* id)
{
    Statement** statementPtr = statementsRegistry.GetPtr(id);
    return ((statementPtr) ? *statementPtr:0);
}
Statement* Connection::getStatement(const char* id)
{
    if (!id || id[0] == '\0') return 0;
    connect();
    MutexGuard guard(statementsRegistryLock);
    return _getStatement(id);
}
Statement* Connection::getStatement(const char* id, const char* sql)
{
    if (!id || id[0] == '\0' || !sql || sql[0] == '\0') return 0;

    connect();

    MutexGuard guard(statementsRegistryLock);
    Statement* statement = _getStatement(id);
    if (!statement)
    {
        statement = createStatement(sql);
        if (statement && !_registerStatement(id, statement)) {
            delete statement; statement = 0;
        }
    }
    return statement;
}

bool Connection::_registerRoutine(const char* id, Routine* routine)
{
    if (!routine || !id || id[0] == '\0' || routinesRegistry.Exists(id)) return false;
    routinesRegistry.Insert(id, routine);
    return true;
}
bool Connection::registerRoutine(const char* id, Routine* routine)
{
    MutexGuard guard(routinesRegistryLock);
    return _registerRoutine(id, routine);
}

bool Connection::unregisterRoutine(const char* id)
{
    if (!id || id[0] == '\0') return false;
    MutexGuard guard(routinesRegistryLock);
    Routine** routinePtr = routinesRegistry.GetPtr(id);
    if (!routinePtr) return false;
    if (*routinePtr) delete *routinePtr;
    routinesRegistry.Delete(id);
    return true;
}
Routine* Connection::_getRoutine(const char* id)
{
    Routine** routinePtr = routinesRegistry.GetPtr(id);
    return ((routinePtr) ? *routinePtr:0);
}
Routine* Connection::getRoutine(const char* id)
{
    if (!id || id[0] == '\0') return 0;
    connect();
    MutexGuard guard(routinesRegistryLock);
    return _getRoutine(id);
}
Routine* Connection::getRoutine(const char* id, const char* call, bool func)
{
    if (!id || id[0] == '\0' || !call || call[0] == '\0') return 0;

    connect();

    MutexGuard guard(routinesRegistryLock);
    Routine* routine = _getRoutine(id);
    if (!routine)
    {
        routine = createRoutine(call, func);
        if (routine && !_registerRoutine(id, routine)) {
            delete routine; routine = 0;
        }
    }
    return routine;
}


void Connection::disconnect()
{
    {
        MutexGuard guard(statementsRegistryLock);

        char* key = 0; Statement* statement = 0; statementsRegistry.First();
        while (statementsRegistry.Next(key, statement))
            if (statement) delete statement;
        
        statementsRegistry.Empty();
    }
    {
        MutexGuard guard(routinesRegistryLock);

        char* key = 0; Routine* routine = 0; routinesRegistry.First();
        while (routinesRegistry.Next(key, routine))
            if (routine) delete routine;
        
        routinesRegistry.Empty();
    }
}

void ConnectionPool::loadPoolSize(ConfigView* config)
{
    __require__(config);

    const char* sizeParam = "connections";
    try 
    {
        size = (unsigned)config->getInt(sizeParam); 
        if (!size || 
            size > SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE_LIMIT)
        {
            size = SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE;
            smsc_log_warn(log, "Maximum ConnectionPool size is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <%s.%s> "
                     "Using default: %u",
                     SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE_LIMIT,
                     config->getInstance(), sizeParam,
                     SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE);
        }
    } 
    catch (ConfigException& exc) 
    {
        size = SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE;
        smsc_log_warn(log, "Maximum ConnectionPool size wasn't specified ! "
                 "Config parameter: <%s.%s> "
                 "Using default: %u", 
                 config->getInstance(), sizeParam,
                 SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE);
    }
}

ConnectionPool::ConnectionPool(DataSource& _ds, ConfigView* config)
    throw(ConfigException) : ds(_ds), count(0),
        log(Logger::getInstance("smsc.db.ConnectionPool")),
            idleHead(0), idleTail(0), idleCount(0), head(0), tail(0), queueLen(0)
{
    loadPoolSize(config);
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
    closeConnections();
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
        pthread_cond_init(&queue.condition,NULL);
        if (!head) head = tail;
        queue.connection = 0L;
        monitor.wait(&queue.condition);
        pthread_cond_destroy(&queue.condition);
        return queue.connection;
    }
    
    Connection* connection = 0L;
    if (idleCount)
    {
        connection = pop();
    }
    else if (count < size) 
    {
        connection = ds.newConnection();
        (void) connections.Push(connection);
        count++;
    }
    return connection; 
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
        if (count > 0) count--;
    }
    else 
    {
        push(connection);
    }
}
void ConnectionPool::closeConnections()
{
    MutexGuard  guard(monitor);
    
    while (connections.Count()) {
        Connection* connection=0L;
        (void) connections.Pop(connection);
        if (connection) delete connection;
    }

    idleHead = 0; idleTail = 0; idleCount = 0; 
    count = 0; head = 0; tail = 0; queueLen = 0;
}
void ConnectionPool::closeRegisteredQueries(const char* id)
{
    MutexGuard  guard(monitor);

    for (int i=0; i<connections.Count(); i++) {
        Connection* connection = connections[i];
        if (!connection) continue;
        connection->unregisterStatement(id);
        connection->unregisterRoutine(id);
    }
}

/* ------------------- Connection WatchDog (DataSource) ------------------- */

void DataSource::init(ConfigView* config) 
    throw(ConfigException)
{
    bool bWdIsNeeded = false;
    try 
    {
        bWdIsNeeded = config->getBool("watchdog", "WatchDog disabled.");
    } 
    catch (ConfigException& exc) {}

    if (bWdIsNeeded) watchDog = new WatchDog();
}

void WatchDog::Start()
{
    MutexGuard  guard(startLock);
    
    if (!bStarted)
    {
        MutexGuard  tmguard(timersLock);
        timers.clear();
        bNeedExit = false;
        Thread::Start();
        bStarted = true;
    }
}
void WatchDog::Stop()
{
    MutexGuard  guard(startLock);

    if (bStarted)
    {
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        bNeedExit = false;
    }
}
int WatchDog::Execute()
{
    while (!bNeedExit)
    {
        timersLock.Lock();
        awake.Wait(0);
        if (timers.empty())
        {
            timersLock.Unlock();
            __trace__("DS WatchDog> Idle");
            awake.Wait(3600*1000); // idle timeout;
            __trace__("DS WatchDog> Idle quit");
        }
        else
        {
            TimersIterator it = timers.begin();
            int timer = it->first;
            ConnectionDeadline cd = it->second;
            int32_t left = cd.deadline - time(NULL);
            if (left <=0)
            {
                __trace2__("DS WatchDog> Timer #%u expired for connection %x. "
                           "Time: left=%d, sleep=%u, current=%u",
                           timer, cd.connection, left, cd.deadline, time(NULL));
                timers.erase(timer);
                timersLock.Unlock();

                try 
                {
                    if (cd.connection) cd.connection->abort();
                    __trace2__("DS WatchDog> DS operation terminated "
                               "on connection %x. ", cd.connection);
                }
                catch (Exception& exc) {
                    __trace2__("DS WatchDog> Termination of DS operation "
                               "on connection %x failed! Cause:",
                               cd.connection, exc.what());
                }
                catch (...) {
                    __trace2__("DS WatchDog> Termination of DS operation "
                               "on connection %x failed! Unknown reason.",
                               cd.connection);
                }
            }
            else
            {
                __trace2__("DS WatchDog> Waiting timer #%u for connection %x. "
                           "Time: left=%d, sleep=%u, current=%u",
                           timer, cd.connection, left, cd.deadline, time(NULL));
                timersLock.Unlock();
                awake.Wait(left*1000);
            }
        }
    }
    exited.Signal();
    return 0;
}

int WatchDog::startTimer(Connection* connection, uint32_t timeout)
{
    {
        MutexGuard  guard(startLock);
        if (!bStarted || timeout == 0) return -1;
    }
    
    MutexGuard  guard(timersLock);
    int timer = (timers.empty()) ? 0:(timers.end()->first+1);
    timers.insert(TimersPair(timer, 
                    ConnectionDeadline(connection, time(NULL)+timeout)));
    __trace2__("DS WatchDog> Timer #%u start for connection %x timeout=%d",
                timer, connection, timeout);
    awake.Signal();
    return timer;
}
void WatchDog::stopTimer(int timer)
{
    {
        MutexGuard  guard(startLock);
        if (!bStarted || timer < 0) return;
    }
    
    __trace2__("DS WatchDog> Timer #%u stop", timer);
    MutexGuard  guard(timersLock);
    timers.erase(timer);
}


}}

