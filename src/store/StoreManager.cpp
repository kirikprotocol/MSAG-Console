#include <stdio.h>
#include <stdlib.h>

#include <orl.h>
#include <util/debug.h>
#include "StoreManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;

MessageStore* StoreManager::instance = 0L;

MessageStore* StoreManager::getInstance() 
	throw(ConnectFailureException)
{
    return ((instance) ? instance : (instance = new StoreManager()));
}

StoreManager::StoreManager() 
	throw(ConnectFailureException)
{
    // It is needed to obtain init parameters from config here !

	StoreConfig* config = new StoreConfig("ORJP", "smsc", "smsc");
    pool = new ConnectionPool(config, 100, 10);
}

StoreManager::~StoreManager()
{
    if (pool) delete pool;
}

SMSId StoreManager::store(SMS& sms) 
	throw(StorageException)
{
	__require__(pool);

    while (1) 
	{
		try 
		{
			Connection* conn = pool->getConnection();
			SMSId id = conn->store(sms);
            pool->freeConnection(conn);
			return id;
		} 
		catch (ConnectFailureException& exc) {
			throw;
		}
		catch (StorageException& exc) {
			printf("Storage Exception : %s\n", exc.what());
			continue;
		}
	}
}

SMS& StoreManager::retrive(SMSId id) 
	throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
	while (1) 
	{
		try 
		{
			Connection* conn = pool->getConnection();
			SMS& sms = conn->retrive(id);
            pool->freeConnection(conn);
			return sms;
		}
		catch (ConnectFailureException& exc) {
			throw;
		}
		catch (NoSuchMessageException& exc) {
			throw;
		}
		catch (StorageException& exc) {
			printf("Storage Exception : %s\n", exc.what());
			continue;
		}
	}
}
/* ----------------------------- StoreManager -------------------------- */

}}

