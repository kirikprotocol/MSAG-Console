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
    return 0;
}

SMS* StoreManager::retrive(SMSId id)
    throw(ResourceAllocationException, NoSuchMessageException)
{
    return 0L;
}
/* ----------------------------- StoreManager -------------------------- */

}}

