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

const int MAX_TRIES_TO_PROCESS = 3;

SMSId StoreManager::store(SMS& sms) 
	throw(StorageException)
{
	__require__(pool && generator);
	
	int iteration=1;
    while(true)
	{
		try 
		{
			Connection* conn = pool->getConnection();
			SMSId id = generator->getNextId();
			conn->store(sms, id);
            pool->freeConnection(conn);
			return id;
		} 
		catch (ConnectionFailedException& exc) {
			throw;
		}
		catch (StorageException& exc) 
		{
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
		try 
		{
			Connection* conn = pool->getConnection();
			SMS& sms = conn->retrive(id);
            pool->freeConnection(conn);
			return sms;
		}
		catch (ConnectionFailedException& exc) {
			throw;
		}
		catch (NoSuchMessageException& exc) {
			throw;
		}
		catch (StorageException& exc) 
		{
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

