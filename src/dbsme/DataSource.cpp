
#include "DataSource.h"

namespace smsc { namespace dbsme
{
using smsc::util::Logger;

/* --------- Abstract Connection Management (ConnectionPool) --------------- */

Hash<DataSourceFactory *>*  DataSourceFactory::registry = 0;

const unsigned SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE = 10;
const unsigned SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE_LIMIT = 1000;

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
            log.warn("Maximum ConnectionPool size is incorrect "
                     "(should be between 1 and %u) ! "
                     "Config parameter: <%s.%s> "
                     "Using default: %u",
                     SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE_LIMIT,
                     config->getCategory(), sizeParam,
                     SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE);
        }
    } 
    catch (ConfigException& exc) 
    {
        size = SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE;
        log.warn("Maximum ConnectionPool size wasn't specified ! "
                 "Config parameter: <%s.%s> "
                 "Using default: %u", 
                 config->getCategory(), sizeParam,
                 SMSC_DS_DEFAULT_CONNECTION_POOL_SIZE);
    }
}

ConnectionPool::ConnectionPool(DataSource& _ds, ConfigView* config)
    throw(ConfigException) : ds(_ds), count(0),
        log(Logger::getCategory("smsc.dbsme.ConnectionPool")),
            head(0L), tail(0L), queueLen(0)
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
    MutexGuard  guard(monitor);

    while (connections.Count())
    {
        Connection* connection=0L;
        (void) connections.Pop(connection);
        if (connection) delete connection;
    }
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
        count--;
    }
    else 
    {
        push(connection);
    }
}

}}

