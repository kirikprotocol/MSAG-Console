#include <stdio.h>
#include <stdlib.h>

#include <orl.h>
#include <util/debug.h>
#include "StoreManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;

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

SMSId StoreManager::store(SMS& sms) 
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
			// Write log here
			printf("Storage Exception : %s\n", exc.what());
			if (iteration < MAX_TRIES_TO_PROCESS) 
			{
				iteration++;
				continue;
            }
			printf("Max tries count exceeded !\n");
			throw;
		}
	}
}

SMS& StoreManager::retrive(SMSId id) 
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
			SMS& sms = connection->retrive(id);
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
			// Write log here
            printf("Storage Exception : %s\n", exc.what());
			if (iteration < MAX_TRIES_TO_PROCESS) 
			{
				iteration++;
				continue;
            }
            printf("Max tries count exided !\n");
			throw;
		}
	}
}
/* ----------------------------- StoreManager -------------------------- */

}}

