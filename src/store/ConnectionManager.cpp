#include <stdlib.h>
#include <string.h>

#include <oci.h>
#include "ConnectionManager.h"

namespace smsc { namespace store 
{
/* ----------------------------- StoreConfig --------------------------- */
StoreConfig::StoreConfig(const char* db, const char* usr, const char* pwd)
{
    userName = strdup(usr);
    userPwd = strdup(pwd);
    dbName = strdup(db);
}
StoreConfig::~StoreConfig() 
{
    if (userName) free(userName);
    if (userPwd) free(userPwd);
    if (dbName) free(dbName);
}
/* ----------------------------- StoreConfig --------------------------- */

/* ----------------------------- ConnectionPool ------------------------ */
SingleConnectionPool::SingleConnectionPool(StoreConfig* _config)
    throw(ResourceAllocationException, AuthenticationException)
        : ConnectionPool(_config), conn(0L), lock(false)
{
    //create mutex here
    connect();
}

SingleConnectionPool::~SingleConnectionPool()
{
    disconnect();
    // destory mutex here
}

void SingleConnectionPool::connect()
    throw(ResourceAllocationException, AuthenticationException)
{
    if (!conn) 
    {
        if (!config) {
            throw AuthenticationException();
        }
        if (!(conn = new Connection(0))) {
            throw ResourceAllocationException();
        }
             
        const char* userName = config->getUserName();
        const char* userPwd = config->getUserPwd();
        const char* dbName = config->getDbName();

        if (userName && userPwd && dbName)
        {
            sword       status;

            /* initialize the mode to be the threaded environment */
            status = OCIEnvCreate(&(conn->envhp), OCI_THREADED, (dvoid *)0,
                                   0, 0, 0, (size_t) 0, (dvoid **)0);
            if (status != OCI_SUCCESS || !(conn->envhp)) 
            {
                throw ResourceAllocationException();
            }

            /* allocate an error handle */
            status = OCIHandleAlloc ((dvoid *)(conn->envhp), 
                                     (dvoid **)&(conn->errhp),
                                      OCI_HTYPE_ERROR, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !(conn->errhp)) 
            {
                throw ResourceAllocationException();
            }
        
            // logon to server database (allocate a service handle) 
            status = OCILogon(conn->envhp, conn->errhp, &(conn->svchp),
                                (OraText*)userName, strlen(userName),
                                (OraText*)userPwd, strlen(userPwd),
                                (OraText*)dbName, strlen(dbName));
            if (status != OCI_SUCCESS || !(conn->svchp))
            {
                // free envirounment handle (error handle will be freed too)
                //checkerror(conn.errhp, status);
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw AuthenticationException();
            }
        } 
        else {
            throw AuthenticationException();
        }
    }
}
 
void SingleConnectionPool::disconnect()
{
    if (conn && conn->envhp && conn->errhp && conn->svchp)
    {
        // logoff from database server
        (void) OCILogoff(conn->svchp, conn->errhp);
        
        // free envirounment handle (error & service handles will be freed too)
        (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
        delete conn; conn = 0L;
    }
}

Connection* SingleConnectionPool::getConnection()
{
    while (lock); // replace it by mutex logic (own mutex here)
    lock = true;
    return conn;
}

void SingleConnectionPool::freeConnection(Connection* connection)
{
    // replace it by mutex logic (free mutex here)
    if (conn == connection) {
        lock = false;
    }
}
/* ----------------------------- ConnectionPool ------------------------ */

}}

