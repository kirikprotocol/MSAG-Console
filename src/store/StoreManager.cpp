#include "StoreManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;

MessageStore* StoreManager::instance = 0L;

MessageStore* StoreManager::getInstance()
    throw(ResourceAllocationException, AuthenticationException)
{
    return ((instance) ? instance : (instance = new StoreManager()));
}

StoreManager::StoreManager()
    throw(ResourceAllocationException, AuthenticationException)
{
    StoreConfig* config = new StoreConfig("ORCL", "smsc", "smsc");
    pool = new SingleConnectionPool(config);
}

StoreManager::~StoreManager()
{
    if (pool) delete pool;
}

SMSId StoreManager::store(SMS* message)
    throw(ResourceAllocationException)
{
    Connection* conn;
    if (pool && (conn = pool->getConnection())) 
    {
          
        pool->freeConnection(conn);
        return 0;
    }
    throw ResourceAllocationException();
}

SMS* StoreManager::retrive(SMSId id)
    throw(ResourceAllocationException, NoSuchMessageException)
{
    Connection* conn;
    SMS*        sms;
    if (pool && (conn = pool->getConnection())) 
    {
          
        pool->freeConnection(conn);
        return sms;
    }
    throw ResourceAllocationException();
}
/* ----------------------------- StoreManager -------------------------- */

}}

