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

SMSId StoreManager::store(SMS* sms)
    throw(ResourceAllocationException)
{
    Connection* conn;
    if (sms && pool && (conn = pool->getConnection())) 
    {
        sword status;
        
        // start transaction
        status = OCITransStart (conn->svchp, conn->errhp, 0, OCI_TRANS_NEW);

        // execute prepared statements
        status = OCIStmtExecute(conn->svchp, conn->storeStmthps[0], conn->errhp,
                                (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT);
          
        status = OCITransCommit (conn->svchp, conn->errhp, OCI_DEFAULT);

        pool->freeConnection(conn);
        return 0;
    }
    throw ResourceAllocationException();
}

SMS* StoreManager::retrive(SMSId id)
    throw(ResourceAllocationException, NoSuchMessageException)
{
    Connection* conn;
    if (pool && (conn = pool->getConnection())) 
    {
        SMS* sms = new SMS();

        pool->freeConnection(conn);
        return sms;
    }
    throw ResourceAllocationException();
}
/* ----------------------------- StoreManager -------------------------- */

}}

