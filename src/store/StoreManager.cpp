#include <stdio.h>
#include <stdlib.h>

#include <orl.h>
#include <util/debug.h>
#include "StoreManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;
using smsc::util::Logger;
using smsc::util::config::Manager;

const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION = 3;

Mutex StoreManager::mutex;
ConnectionPool* StoreManager::pool = 0L;
IDGenerator* StoreManager::generator = 0L;
StoreManager* StoreManager::instance = 0L;
unsigned StoreManager::maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");

void StoreManager::loadMaxTriesCount(Manager& config)
{
    try 
    {
        maxTriesCount = (unsigned)config.getInt("MessageStore.maxTriesCount");
    } 
    catch (ConfigException& exc) 
    {
        maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
        log.warn("Max tries count to process operation wasn't specified ! "
                 "Using default: %d", maxTriesCount);
    }
}

void StoreManager::startup(Manager& config)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);
    
    if (!instance)
    {
        log.info("Storage Manager is starting ... ");
        Connection* connection = 0L; 
        loadMaxTriesCount(config);
        try 
        {
            pool = new ConnectionPool(config);
            connection = pool->getConnection();
            generator = new IDGenerator(connection->getMessagesCount());
        }
        // catch (TooLargeQueueException) ???
        catch (StoreException& exc)
        {
            log.error("StoreException: %s", exc.what());
            if (pool) 
            {
                if (connection)
                {
                    pool->freeConnection(connection);
                }
                delete pool; pool = 0L;
            }
            if (generator)
            {
                delete generator; generator = 0L;
            }
            throw ConnectionFailedException(exc);
        }
        pool->freeConnection(connection);
        instance = new StoreManager();
        log.info("Storage Manager was started up.");
    }
}
        
void StoreManager::shutdown() 
{
    MutexGuard guard(mutex);

    if (pool && instance && generator)
    {
        log.info("Storage Manager is shutting down ...");
        delete pool; pool = 0L;
        delete instance; instance = 0L;
        delete generator; generator = 0L;
        log.info("Storage Manager was shutdowned.");
    }   
}

SMSId StoreManager::store(const SMS &sms) 
    throw(StorageException)
{
    __require__(pool && generator);
    
    SMSId id = generator->getNextId();
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            connection->store(sms, id);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            continue;
        }
        catch (TooLargeQueueException& exc)
        {
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount) 
            {
                log.warn("Max tries count to store message "
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }
    return id;
}

void StoreManager::retrive(SMSId id, SMS &sms) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            connection->retrive(id, sms);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            continue;
        }
        catch (TooLargeQueueException& exc)
        {
            throw;
        }
        catch (NoSuchMessageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount) 
            {
                log.warn("Max tries count to retrive message "
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }
}

void StoreManager::remove(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            connection->remove(id);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            continue;
        }
        catch (TooLargeQueueException& exc)
        {
            throw;
        }
        catch (NoSuchMessageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount) 
            {
                log.warn("Max tries count to remove message "
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }
}

void StoreManager::replace(SMSId id, const SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            connection->replace(id, sms);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            continue;
        }
        catch (TooLargeQueueException& exc)
        {
            throw;
        }
        catch (NoSuchMessageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount) 
            {
                log.warn("Max tries count to replace message "
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }
}

/* ----------------------------- StoreManager -------------------------- */

}}

