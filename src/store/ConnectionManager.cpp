#include <stdio.h>
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
static text* sqlStoreStmt0 = (text *)
"UPDATE SMS_ID_LOCK SET ID=0 WHERE TGT='SMS_MSG_TABLE'";

static text* sqlStoreStmt1 = (text *)
"SELECT MAX(ID) FROM SMS_MSG";
            
static text* sqlStoreStmt2 = (text *)
"INSERT INTO SMS_MSG VALUES (:ID, :ST, :MR, :RM,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :SRR, :RD, :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";

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
            
            (void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,                         
                                 (dvoid * (*)(dvoid *, size_t)) 0,                      
                                 (dvoid * (*)(dvoid *, dvoid *, size_t))0,              
                                 (void (*)(dvoid *, dvoid *)) 0 );                      
                                                                              
            (void) OCIEnvInit((OCIEnv **) &(conn->envhp), OCI_DEFAULT, (size_t) 0,             
                                 (dvoid **) 0 );                                           
                                                                              
            (void) OCIHandleAlloc((dvoid *)(conn->envhp), (dvoid **)&(conn->errhp),
                                   OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);                                 
                                                                              
            (void) OCIHandleAlloc((dvoid *)(conn->envhp), (dvoid **) &(conn->srvhp),
                                   OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0);                                 
                                                                              
            (void) OCIHandleAlloc((dvoid *)(conn->envhp), (dvoid **)&(conn->svchp),
                                   OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0);                                 
                                                                              
            (void) OCIServerAttach(conn->srvhp, conn->errhp, 
                                   (text *)dbName, strlen(dbName), 0);           
                                                                              
            (void) OCIAttrSet((dvoid *)(conn->svchp), OCI_HTYPE_SVCCTX, 
                              (dvoid *)(conn->srvhp), (ub4) 0, 
                              OCI_ATTR_SERVER, (OCIError *)(conn->errhp));            
                                                                              
            (void) OCIHandleAlloc((dvoid *)(conn->envhp), (dvoid **)&(conn->sesshp),
                                  (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);   
                                                                              
            (void) OCIAttrSet((dvoid *)(conn->sesshp), (ub4) OCI_HTYPE_SESSION,                 
                              (dvoid *)userName, (ub4)strlen(userName),          
                              (ub4) OCI_ATTR_USERNAME, conn->errhp);                             
                                                                              
            (void) OCIAttrSet((dvoid *)(conn->sesshp), (ub4) OCI_HTYPE_SESSION,                 
                              (dvoid *)userPwd, (ub4) strlen(userPwd),          
                              (ub4) OCI_ATTR_PASSWORD, conn->errhp);                             
                                                                              
            status = OCISessionBegin (conn->svchp, conn->errhp, conn->sesshp,
                                      OCI_CRED_RDBMS, (ub4) OCI_DEFAULT); 
            if (status != OCI_SUCCESS) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw AuthenticationException();
            }
                                                                              
            (void) OCIAttrSet((dvoid *)(conn->svchp), (ub4) OCI_HTYPE_SVCCTX,                  
                              (dvoid *)(conn->sesshp), (ub4) 0, 
                              (ub4) OCI_ATTR_SESSION, conn->errhp);                            

            // ----------- Prepare statements here --------------

            // allocate statements handles
            status = OCIHandleAlloc ((dvoid *)(conn->envhp), 
                                     (dvoid **)&(conn->storeStmthps[0]),
                                     OCI_HTYPE_STMT, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !(conn->storeStmthps[0])) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
            }
            status = OCIHandleAlloc ((dvoid *)(conn->envhp), 
                                     (dvoid **)&(conn->storeStmthps[1]),
                                     OCI_HTYPE_STMT, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !(conn->storeStmthps[1])) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
            }
            status = OCIHandleAlloc ((dvoid *)(conn->envhp), 
                                     (dvoid **)&(conn->storeStmthps[2]),
                                     OCI_HTYPE_STMT, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !(conn->storeStmthps[2])) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
            }
            
            status =  OCIStmtPrepare(conn->storeStmthps[0], conn->errhp, sqlStoreStmt0,
                                    (ub4)strlen((char *)sqlStoreStmt0),
                                    (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
            if (status != OCI_SUCCESS) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
            }
            status = OCIStmtPrepare(conn->storeStmthps[1], conn->errhp, sqlStoreStmt1,
                                    (ub4)strlen((char *)sqlStoreStmt1),
                                    (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
            if (status != OCI_SUCCESS) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
            }
            status = OCIStmtPrepare(conn->storeStmthps[2], conn->errhp, sqlStoreStmt2,
                                    (ub4)strlen((char *)sqlStoreStmt2),
                                    (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
            if (status != OCI_SUCCESS) 
            {
                (void) OCIHandleFree(conn->envhp, OCI_HTYPE_ENV);
                throw ResourceAllocationException();
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

