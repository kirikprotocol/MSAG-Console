#include <stdio.h>
#include <stdlib.h>

#include <orl.h>
#include "StoreManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;

MessageStore* StoreManager::instance = 0L;

MessageStore* StoreManager::getInstance() throw(StoreException)
{
    return ((instance) ? instance : (instance = new StoreManager()));
}

StoreManager::StoreManager() throw(StoreException)
{
    StoreConfig* config = new StoreConfig("ORJP", "smsc", "smsc");
    pool = new ConnectionPool(config);
}

StoreManager::~StoreManager()
{
    if (pool) delete pool;
}

SMSId StoreManager::store(SMS& sms) throw(StoreException)
{
    Connection* conn;
    if (pool && (conn = pool->getConnection())) 
    {
        SMSId id = conn->store(sms);

		pool->freeConnection(conn);
        return id;
    }
    throw ResourceAllocationException();
}

SMS& StoreManager::retrive(SMSId id) throw(StoreException)
{
    Connection* conn;
    if (pool && (conn = pool->getConnection())) 
    {
        SMS& sms = conn->retrive(id);

        pool->freeConnection(conn);
        return sms;
    }
    throw ResourceAllocationException();
}
/* ----------------------------- StoreManager -------------------------- */

}}

