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
const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT = 1000;

Mutex StoreManager::mutex;
Archiver* StoreManager::archiver = 0L;
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
        if (!maxTriesCount || 
            maxTriesCount > SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT)
        {
            maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION; 
            log.warn("Max tries count to process operation on MessageStore "
                     "is incorrect (should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.maxTriesCount> "
                     "Using default: %u",
                     SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT,
                     SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
        }
    } 
    catch (ConfigException& exc) 
    {
        maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
        log.warn("Max tries count to process operation on MessageStore "
                 "wasn't specified ! "
                 "Config parameter: <MessageStore.maxTriesCount> "
                 "Using default: %d",
                 SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
    }
}

void StoreManager::startup(Manager& config)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);
    
    if (!instance)
    {
        log.info("Storage Manager is starting ... ");
        loadMaxTriesCount(config);
        try 
        {
            pool = new ConnectionPool(config);
            archiver = new Archiver(config);
            generator = new IDGenerator(archiver->getMaxId());
        }
        catch (StorageException& exc)
        {
            //log.error("StorageException: %s", exc.what());
            if (pool) 
            {
                delete pool; pool = 0L;
            }
            if (generator)
            {
                delete generator; generator = 0L;
            }
            throw ConnectionFailedException(exc);
        }
        instance = new StoreManager();
        archiver->Start();
        log.info("Storage Manager was started up.");
    }
}
        
void StoreManager::shutdown() 
{
    MutexGuard guard(mutex);

    if (pool && instance && generator && archiver)
    {
        log.info("Storage Manager is shutting down ...");
        delete pool; pool = 0L;
        delete instance; instance = 0L;
        delete archiver; archiver = 0L;
        delete generator; generator = 0L;
        log.info("Storage Manager was shutdowned.");
    }   
}

SMSId StoreManager::store(const SMS &sms) 
    throw(StorageException, DuplicateMessageException)
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
        catch (DuplicateMessageException& exc) 
        {
            if (connection) pool->freeConnection(connection);
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
            // state ??? archiver->decrementFinalizedCount();
            pool->freeConnection(connection);
            break;
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

void StoreManager::update(SMSId id, const State state, 
                          time_t operationTime, uint8_t fcs) 
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
            connection->update(id, state, operationTime, fcs);
            if (state != ENROUTE)
            {
                archiver->incrementFinalizedCount();
            }
            pool->freeConnection(connection);
            break;
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
                log.warn("Max tries count to update message "
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }
}

}}

