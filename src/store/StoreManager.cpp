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

Mutex StoreManager::mutex;
ConnectionPool* StoreManager::pool = 0L;
IDGenerator* StoreManager::generator = 0L;
StoreManager* StoreManager::instance = 0L;
log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");

void StoreManager::startup(Manager& config)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);
    
    if (!instance)
    {
        log.info("Storage Manager is starting ... ");
        Connection* connection = 0L; 
        try 
        {
            pool = new ConnectionPool(config);
            connection = pool->getConnection();
            generator = new IDGenerator(connection->getMessagesCount());
        }
        catch (StorageException& exc) 
        {
            log.error("Storage Exception : %s\n", exc.what());
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

const int MAX_TRIES_TO_PROCESS = 3;

SMSId StoreManager::store(const SMS &sms) 
    throw(StorageException)
{
    __require__(pool && generator);
    
    int iteration=1;
    while(true)
    {
        Connection* connection = 0L;
        try 
        {
            connection = pool->getConnection();
            SMSId id = generator->getNextId();
            connection->store(sms, id);
            pool->freeConnection(connection);
            return id;
        } 
        catch (ConnectionFailedException& exc) {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            log.debug("Storage Exception : %s\n", exc.what());
            if (iteration < MAX_TRIES_TO_PROCESS) 
            {
                iteration++;
                continue;
            }
            log.warn("Max tries count exceeded !\n");
            throw;
        }
    }
}

void StoreManager::retrive(SMSId id, SMS &sms) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    int iteration=1;
    while (true)
    {
        Connection* connection = 0L;
        try 
        {
            connection = pool->getConnection();
            connection->retrive(id, sms);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (NoSuchMessageException& exc) {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            log.debug("Storage Exception : %s\n", exc.what());
            if (iteration < MAX_TRIES_TO_PROCESS) 
            {
                iteration++;
                continue;
            }
            log.warn("Max tries count exceeded !\n");
            throw;
        }
    }
}

void StoreManager::remove(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    int iteration=1;
    while (true)
    {
        Connection* connection = 0L;
        try 
        {
            connection = pool->getConnection();
            connection->remove(id);
            pool->freeConnection(connection);
            break;
        }
        catch (ConnectionFailedException& exc) {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (NoSuchMessageException& exc) {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
            log.debug("Storage Exception : %s\n", exc.what());
            if (iteration < MAX_TRIES_TO_PROCESS) 
            {
                iteration++;
                continue;
            }
            log.warn("Max tries count exceeded !\n");
            throw;
        }
    }
}

/* ----------------------------- StoreManager -------------------------- */

}}

