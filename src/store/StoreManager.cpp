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
ConnectionPool*	StoreManager::pool = 0L;
IDGenerator* StoreManager::generator = 0L;
StoreManager* StoreManager::instance = 0L;

void StoreManager::startup(StoreConfig* config)
	throw(ConnectionFailedException)
{
	MutexGuard guard(mutex);

	if (!instance)
	{
        pool = new ConnectionPool(config);
		Connection* connection = pool->getConnection();
		try
		{
			generator = new IDGenerator(connection->getMessagesCount());
		}
		catch (StorageException& exc) 
		{
			pool->freeConnection(connection);
			throw ConnectionFailedException(exc);
		}
		pool->freeConnection(connection);
		instance = new StoreManager();
	}
}
		
void StoreManager::shutdown() 
{
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

const SMS& StoreManager::retrive(SMSId id) 
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
			const SMS& sms = connection->retrive(id);
            pool->freeConnection(connection);
			return sms;
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

