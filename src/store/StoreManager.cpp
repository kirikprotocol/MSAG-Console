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

Mutex StoreManager::mutex;
ConnectionPool* StoreManager::pool = 0L;
IDGenerator* StoreManager::generator = 0L;
StoreManager* StoreManager::instance = 0L;
log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");

MessageStore* StoreManager::startup(const char* db, const char* user,
                                    const char* password, 
                                    unsigned size, unsigned init)
    throw(ConnectionFailedException)
{
    log.info("Storage Manager is starting ... ");
    MutexGuard guard(mutex);
    
    if (!instance)
    {
        Connection* connection = 0L; 
        try 
        {
            pool = new ConnectionPool(db, user, password, size, init);
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
    }
    
    log.info("Storage Manager was started up.");
    return ((MessageStore *)instance);
}
        
void StoreManager::shutdown() 
{
    log.info("Storage Manager is shutting down ...");
    MutexGuard guard(mutex);

    if (pool) {
        delete pool; pool = 0L;
    }
    if (instance) {
        delete instance; instance = 0L;
    }
    if (generator) {
        delete generator; generator = 0L;
    }
    log.info("Storage Manager was shutdowned.");
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

