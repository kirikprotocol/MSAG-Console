#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oci.h>
#include <orl.h>

#include "ConnectionManager.h"

#include <util/debug.h>
#include <sms/sms.h>

namespace smsc { namespace store 
{
using namespace smsc::sms;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

/* ----------------------------- ConnectionPool ------------------------ */

const unsigned SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE = 10;
const unsigned SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE = 5;

log4cpp::Category& ConnectionPool::log = 
    Logger::getCategory("smsc.store.ConnectionPool");

void ConnectionPool::loadMaxSize(Manager& config)
{
    try {
        size = (unsigned)config.getInt("db.connections.max");
    } catch (ConfigException& exc) {
        size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
        log.warn("ConnectionPool max size wasn't specified ! "
                 "Using default: %d", size);
    }
}

void ConnectionPool::loadInitSize(Manager& config)
{
    try {
        count = (unsigned)config.getInt("db.connections.init");
    } catch (ConfigException& exc) {
        count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
        log.warn("ConnectionPool init size wasn't specified ! "
                 "Using default: %d", count);
    }
}

void ConnectionPool::loadDBInstance(Manager& config)
    throw(ConfigException)
{
    try {
        dbInstance = config.getString("db.instance");   
    } catch (ConfigException& exc) {
        log.error("DB instance name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserName(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserName = config.getString("db.user");   
    } catch (ConfigException& exc) {
        log.error("DB user name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserPassword(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserPassword = config.getString("db.password");   
    } catch (ConfigException& exc) {
        log.error("DB user password wasn't specified !");
        throw;
    }
}

ConnectionPool::ConnectionPool(Manager& config)
    throw(ConfigException, ConnectionFailedException) 
{
    
    loadMaxSize(config);
    loadInitSize(config);
    loadDBInstance(config);
    loadDBUserName(config);
    loadDBUserPassword(config);

    if (size < count) 
    {
        size = count;
        log.warn("Specified ConnectionPool size less than init size. "
                  "Using max: %d", size);
    }

    __require__(dbInstance && dbUserName && dbUserPassword);

    for (int i=0; i<count; i++)
    {
        Connection* connection = new Connection(this);
        (void) idle.Push(connection);
    }
}

ConnectionPool::~ConnectionPool()
{
    MutexGuard  guard(monitor);

    for (int i=0; i<idle.Count(); i++)
    {
        Connection* connection=0L;
        (void) idle.Pop(connection);
        if (connection) delete connection;
    }
    for (int i=0; i<busy.Count(); i++)
    {
        Connection* connection=0L;
        (void) busy.Pop(connection);
        if (connection) delete connection;
    }
    for (int i=0; i<dead.Count(); i++) 
    {
        Connection* connection=0L;
        (void) dead.Pop(connection);
        if (connection) delete connection;
    }
    
    delete dbInstance;
    delete dbUserName;
    delete dbUserPassword;
}

bool ConnectionPool::hasFreeConnections()
{
    return (count<size || idle.Count());
}

Connection* ConnectionPool::getConnection()
    throw(ConnectionFailedException)
{
    MutexGuard  guard(monitor);

    while (!hasFreeConnections()) monitor.wait();
    
    Connection* connection=0L;
    if (idle.Count())
    {
        (void) idle.Pop(connection);
        (void) busy.Push(connection);
    } 
    else if (count < size)
    {
        connection = new Connection(this);
        (void) busy.Push(connection);
        count++;
    }
    
    __require__(connection);
    return connection;
}

void ConnectionPool::freeConnection(Connection* connection)
{
    __require__(connection);
    MutexGuard  guard(monitor);
    
    Connection* tmp=0L;
    for (int i=0; i<busy.Count(); i++)
    {
        tmp = busy[i];
        if (tmp == connection) 
        {
            busy.Delete(i);
            if (count <= size)
            {
                (void) idle.Push(connection);
                monitor.notify();
            }
            else 
            {
                delete connection;
                count--;
            }
            return;
        }
    }
    for (int i=0; i<dead.Count(); i++)
    {
        tmp = dead[i];
        if (tmp == connection) 
        {
            dead.Delete(i);
            delete connection;
            count--;
            monitor.notify();
            return;
        }
    }
}

void ConnectionPool::killConnection(Connection* connection)
{
    __require__(connection);
    MutexGuard  guard(monitor);

    Connection* tmp=0L;
    for (int i=0; i<busy.Count(); i++)
    {
        tmp = busy[i];
        if (tmp == connection) // set Connection dead
        { 
            busy.Delete(i); 
            (void) dead.Push(connection);
            break;
        }
    }
}

void ConnectionPool::setSize(unsigned new_size) 
{
    if (!new_size || new_size == size) return;
    MutexGuard  guard(monitor);
    
    if (new_size < size)
    {
        Connection* connection = 0L;
        for (int i=new_size-1; i<idle.Count(); i++)
        {
            (void) idle.Pop(connection);
            if (connection) delete connection;
            count--;
        }
    }

    size = new_size;
    monitor.notify();
}

/* ----------------------------- ConnectionPool ------------------------ */

/* ------------------------------- Connection -------------------------- */

text* Connection::sqlGetMessagesCount = (text *)
"SELECT NVL(MAX(ID), 0) FROM SMS_MSG";
 
text* Connection::sqlStoreInsert = (text *)
"INSERT INTO SMS_MSG VALUES (:ID, :ST, :MR, :RM,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :SRR, :RD, :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";

text* Connection::sqlRetriveAll = (text *)
"SELECT ST, MR, RM, OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON,\
 DA_NPI, DA_VAL, VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME,\
 SRR, RD, PRI, PID, FCS, DCS, UDHI, UDL, UD FROM SMS_MSG WHERE ID=:ID";

text* Connection::sqlRemove = (text *)
"DELETE FROM SMS_MSG WHERE ID=:ID";

Connection::Connection(ConnectionPool* pool) 
    throw(ConnectionFailedException) 
        : owner(pool), envhp(0L), errhp(0L), svchp(0L), srvhp(0L), sesshp(0L)
{
    __require__(owner);

    const char* dbName = owner->getDBInstance();
    const char* userName = owner->getDBUserName();
    const char* userPwd = owner->getDBUserPassword();
    
    __require__(userName && userPwd && dbName);

    // open connection to DB and begin user session 
    sword status;
    status = OCIEnvCreate(&envhp, OCI_OBJECT|OCI_ENV_NO_MUTEX, 
                          (dvoid *)0, 0, 0, 0, (size_t) 0, (dvoid **)0);
    __require__(status == OCI_SUCCESS);

    status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, 
                            OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
    __require__(status == OCI_SUCCESS && errhp);

    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp,
                                OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)); 
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp,
                                OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0));
    checkConnErr(OCIServerAttach(srvhp, errhp, (text *)dbName,
                                 strlen(dbName), OCI_DEFAULT));
    checkConnErr(OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX,
                            (dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp));
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&sesshp,
                                OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0));   
    checkConnErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                            (dvoid *)userName, (ub4)strlen(userName),
                            (ub4) OCI_ATTR_USERNAME, errhp));
    checkConnErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                            (dvoid *)userPwd, (ub4) strlen(userPwd),
                            (ub4) OCI_ATTR_PASSWORD, errhp));
    checkConnErr(OCISessionBegin(svchp, errhp, sesshp, OCI_CRED_RDBMS,
                                 (ub4) OCI_DEFAULT));
    checkConnErr(OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
                            (dvoid *)sesshp, (ub4) 0,
                            (ub4) OCI_ATTR_SESSION, errhp));

    // allocate statements handles
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtGetMessagesCount,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0));
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtStoreInsert,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0));
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtRetriveAll,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0));
    checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtRemove,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0));

    // prepare statements
    checkConnErr(OCIStmtPrepare(stmtGetMessagesCount, errhp, sqlGetMessagesCount,
                                (ub4)strlen((char *)sqlGetMessagesCount),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    checkConnErr(OCIStmtPrepare(stmtStoreInsert, errhp, sqlStoreInsert,
                                (ub4)strlen((char *)sqlStoreInsert),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    checkConnErr(OCIStmtPrepare(stmtRetriveAll, errhp, sqlRetriveAll,
                                (ub4)strlen((char *)sqlRetriveAll),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    checkConnErr(OCIStmtPrepare(stmtRemove, errhp, sqlRemove,
                                (ub4)strlen((char *)sqlRemove),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    
    // define placeholder for removing
    checkConnErr(OCIBindByPos(stmtRemove, &bndRemoveId, errhp, (ub4) 1,
                              (dvoid *) &smsId, (sb4) sizeof(smsId),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));
    checkConnErr(OCIDefineByPos(stmtRemove, &defRemoveRes, errhp, (ub4) 1,
                                (dvoid *) &(smsId), (sword) sizeof(smsId), 
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, 
                                (ub2 *)0, OCI_DEFAULT));
    
    // define placeholder for max(id)
    checkConnErr(OCIDefineByPos(stmtGetMessagesCount, &defMaxId, errhp, (ub4) 1,
                                (dvoid *) &(smsId), (sword) sizeof(smsId), 
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, 
                                (ub2 *)0, OCI_DEFAULT));

    // bind sms placeholder fields for storing
    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndStoreId, errhp, (ub4) 1,
                              (dvoid *) &smsId, (sb4) sizeof(smsId),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndSt, errhp, (ub4) 2,
                              (dvoid *) &(uState), 
                              (sb4) sizeof(uState), SQLT_UIN, 
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndMsgRef, errhp, (ub4) 3,
                              (dvoid *) &(sms.messageReference),
                              (sb4) sizeof(sms.messageReference), SQLT_UIN,
                              (dvoid *)0,(ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndMsgInd, errhp, (ub4) 4,
                              (dvoid *) &(sms.messageIdentifier),
                              (sb4) sizeof(sms.messageIdentifier), SQLT_UIN,
                              (dvoid *)0, (ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndOALen, errhp, (ub4) 5,
                              (dvoid *) &(sms.originatingAddress.lenght),
                              (sb4) sizeof(sms.originatingAddress.lenght),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndOATon, errhp, (ub4) 6,
                              (dvoid *) &(sms.originatingAddress.type),
                              (sb4) sizeof(sms.originatingAddress.type),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT)); 

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndOANpi, errhp, (ub4) 7,
                              (dvoid *) &(sms.originatingAddress.plan),
                              (sb4) sizeof(sms.originatingAddress.plan),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndOAVal, errhp, (ub4) 8,
                              (dvoid *)(sms.originatingAddress.value),
                              (sb4) sizeof(sms.originatingAddress.value),
                              SQLT_STR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDALen, errhp, (ub4) 9,
                              (dvoid *) &(sms.destinationAddress.lenght),
                              (sb4) sizeof(sms.destinationAddress.lenght),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDATon, errhp, (ub4) 10,
                              (dvoid *) &(sms.destinationAddress.type),
                              (sb4) sizeof(sms.destinationAddress.type),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT)); 

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDANpi, errhp, (ub4) 11,
                              (dvoid *) &(sms.destinationAddress.plan),
                              (sb4) sizeof(sms.destinationAddress.plan),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT)); 

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDAVal, errhp, (ub4) 12,
                              (dvoid *)(sms.destinationAddress.value),
                              (sb4) sizeof(sms.destinationAddress.value),
                              SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndVTime, errhp, (ub4) 13,
                              (dvoid *) &(validTime), 
                              (sb4) sizeof(validTime),
                              SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndWTime, errhp, (ub4) 14,
                              (dvoid *) &(waitTime), 
                              (sb4) sizeof(waitTime),
                              SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndSTime, errhp, (ub4) 15,
                              (dvoid *) &(submitTime), 
                              (sb4) sizeof(submitTime), SQLT_ODT, 
                              (dvoid *)0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDTime, errhp, (ub4) 16,
                              (dvoid *) &(deliveryTime),
                              (sb4) sizeof(deliveryTime), 
                              SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT)); 

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndSrr, errhp, (ub4) 17,
                              (dvoid *) &(bStatusReport),
                              (sb4) sizeof(bStatusReport), SQLT_AFC,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndRd, errhp, (ub4) 18,
                              (dvoid *) &(bRejectDuplicates),
                              (sb4) sizeof(bRejectDuplicates), SQLT_AFC,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndMsgPri, errhp, (ub4) 19,
                              (dvoid *) &(sms.priority), 
                              (sb4) sizeof(sms.priority), SQLT_UIN,
                              (dvoid *)0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndMsgPid, errhp, (ub4) 20,
                              (dvoid *) &(sms.protocolIdentifier),
                              (sb4) sizeof(sms.protocolIdentifier),
                              SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndFcs, errhp, (ub4) 21,
                              (dvoid *) &(sms.failureCause),
                              (sb4) sizeof(sms.failureCause), SQLT_UIN,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndDcs, errhp, (ub4) 22,
                              (dvoid *) &(sms.messageBody.scheme),
                              (sb4) sizeof(sms.messageBody.scheme),
                              SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndUdhi, errhp, (ub4) 23,
                              (dvoid *) &(bHeaderIndicator),
                              (sb4) sizeof(bHeaderIndicator), SQLT_AFC,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndUdl, errhp, (ub4) 24,
                              (dvoid *) &(sms.messageBody.lenght), 
                              (sb4) sizeof(sms.messageBody.lenght), SQLT_UIN,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
                              (ub4 *)0, OCI_DEFAULT));
    
    checkConnErr(OCIBindByPos(stmtStoreInsert, &bndUd, errhp, (ub4) 25,
                              (dvoid *) (sms.messageBody.data),
                              (sb4) sizeof(SMSData), 
                              SQLT_BIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));

    // define placeholders fields for retriving
    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defSt, errhp, (ub4) 1,
                                (dvoid *) &(uState), 
                                (sword) sizeof(uState), SQLT_UIN,
                                (dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defMsgRef, errhp, (ub4) 2,
                                (dvoid *) &(sms.messageReference), 
                                (sword) sizeof(sms.messageReference), SQLT_UIN,
                                (dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defMsgInd, errhp, (ub4) 3,
                                (dvoid *) &(sms.messageIdentifier), 
                                (sword) sizeof(sms.messageIdentifier), SQLT_UIN,
                                (dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defOALen, errhp, (ub4) 4,
                                (dvoid *) &(sms.originatingAddress.lenght), 
                                (sword) sizeof(sms.originatingAddress.lenght),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defOATon, errhp, (ub4) 5,
                                (dvoid *) &(sms.originatingAddress.type), 
                                (sword) sizeof(sms.originatingAddress.type),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defOANpi, errhp, (ub4) 6,
                                (dvoid *) &(sms.originatingAddress.plan), 
                                (sword) sizeof(sms.originatingAddress.plan),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defOAVal, errhp, (ub4) 7,
                                (dvoid *)(sms.originatingAddress.value), 
                                (sword) sizeof(sms.originatingAddress.value),
                                SQLT_STR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDALen, errhp, (ub4) 8,
                                (dvoid *) &(sms.destinationAddress.lenght), 
                                (sword) sizeof(sms.destinationAddress.lenght),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDATon, errhp, (ub4) 9,
                                (dvoid *) &(sms.destinationAddress.type), 
                                (sword) sizeof(sms.destinationAddress.type),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDANpi, errhp, (ub4) 10,
                                (dvoid *) &(sms.destinationAddress.plan), 
                                (sword) sizeof(sms.destinationAddress.plan),
                                SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDAVal, errhp, (ub4) 11,
                                (dvoid *)(sms.destinationAddress.value), 
                                (sword) sizeof(sms.destinationAddress.value),
                                SQLT_STR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
                                OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defVTime, errhp, (ub4) 12,
                                (dvoid *) &(validTime), 
                                (sword) sizeof(validTime), SQLT_ODT,
                                (dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defWTime, errhp, (ub4) 13,
                                (dvoid *) &(waitTime), 
                                (sb4) sizeof(waitTime), SQLT_ODT, 
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defSTime, errhp, (ub4) 14,
                                (dvoid *) &(submitTime), 
                                (sb4) sizeof(submitTime), SQLT_ODT,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDTime, errhp, (ub4) 15,
                                (dvoid *) &(deliveryTime), 
                                (sb4) sizeof(deliveryTime), SQLT_ODT,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defSrr, errhp, (ub4) 16,
                                (dvoid *) &(bStatusReport), 
                                (sb4) sizeof(bStatusReport), SQLT_AFC,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defRd, errhp, (ub4) 17,
                                (dvoid *) &(bRejectDuplicates), 
                                (sb4) sizeof(bRejectDuplicates), SQLT_AFC,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defMsgPri, errhp, (ub4) 18,
                                (dvoid *) &(sms.priority), 
                                (sb4) sizeof(sms.priority), SQLT_UIN,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defMsgPid, errhp, (ub4) 19,
                                (dvoid *) &(sms.protocolIdentifier), 
                                (sb4) sizeof(sms.protocolIdentifier), SQLT_UIN,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defFcs, errhp, (ub4) 20,
                                (dvoid *) &(sms.failureCause), 
                                (sb4) sizeof(sms.failureCause), SQLT_UIN,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defDcs, errhp, (ub4) 21,
                                (dvoid *) &(sms.messageBody.scheme), 
                                (sb4) sizeof(sms.messageBody.scheme), SQLT_UIN,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defUdhi, errhp, (ub4) 22,
                                (dvoid *) &(bHeaderIndicator), 
                                (sb4) sizeof(bHeaderIndicator), SQLT_AFC,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defUdl, errhp, (ub4) 23,
                                (dvoid *) &(sms.messageBody.lenght), 
                                (sb4) sizeof(sms.messageBody.lenght), SQLT_UIN,
                                (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
    
    checkConnErr(OCIDefineByPos(stmtRetriveAll, &defUd, errhp, (ub4) 24,
                                (dvoid *) (sms.messageBody.data), 
                                (sb4) sizeof(SMSData), 
                                SQLT_BIN, (dvoid *)0, (ub2 *)0, 
                                (ub2 *)0, OCI_DEFAULT));

    checkConnErr(OCIBindByPos(stmtRetriveAll, &bndRetriveId, errhp, (ub4) 1,
                              (dvoid *) &(smsId), (sb4) sizeof(smsId), 
                              SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
                              (ub4)0, (ub4 *)0, OCI_DEFAULT));
}

Connection::~Connection()
{
    MutexGuard  guard(mutex);

    __require__(envhp && errhp && svchp);
    
    // logoff from database server
    (void) OCILogoff(svchp, errhp);

    // free envirounment handle (other handles will be freed too)
    (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
}

SMSId Connection::getMessagesCount()
    throw(ConnectionFailedException)
{
    MutexGuard  guard(mutex);

    checkConnErr(OCIStmtExecute(svchp, stmtGetMessagesCount, errhp, 
                                (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT));
    return smsId;
}

void Connection::store(const SMS &_sms, SMSId id) 
    throw(StorageException)
{
    MutexGuard  guard(mutex);

    smsId = id;
    sms = _sms;
    
    tm* dt;

    dt = localtime(&(sms.validTime));
    OCIDateSetDate(&validTime, (sb2)(1900+dt->tm_year), 
                   (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
    OCIDateSetTime(&validTime, (ub1)(dt->tm_hour), 
                   (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
    
    dt = localtime(&(sms.waitTime));
    OCIDateSetDate(&waitTime, (sb2)(1900+dt->tm_year), 
                   (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
    OCIDateSetTime(&waitTime, (ub1)(dt->tm_hour), 
                   (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
    
    dt = localtime(&(sms.submitTime));
    OCIDateSetDate(&submitTime, (sb2)(1900+dt->tm_year), 
                   (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
    OCIDateSetTime(&submitTime, (ub1)(dt->tm_hour), 
                   (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
    
    dt = localtime(&(sms.deliveryTime));
    OCIDateSetDate(&deliveryTime, (sb2)(1900+dt->tm_year), 
                   (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
    OCIDateSetTime(&deliveryTime, (ub1)(dt->tm_hour), 
                   (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
    
    bStatusReport = sms.statusReportRequested ? 'Y':'N';
    bRejectDuplicates = sms.rejectDuplicates ? 'Y':'N';
    bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
    uState = (uint8_t) sms.state;
    
    try 
    {
        // insert new sms row into table
        checkErr(OCIStmtExecute(svchp, stmtStoreInsert, errhp, (ub4) 1, (ub4) 0,
                                (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                                OCI_DEFAULT));
        
        checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
    } 
    catch (StorageException& exc) 
    {
        checkErr(OCITransRollback(svchp, errhp, OCI_DEFAULT));
        throw exc;
    }
}

void Connection::retrive(SMSId id, SMS &_sms) 
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);

    smsId = id;
    sword status;
    // retrive entire sms row from table
    status = OCIStmtExecute(svchp, stmtRetriveAll, errhp, (ub4) 1, (ub4) 0,
                            (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                            OCI_DEFAULT);
    if (status == OCI_NO_DATA)
    {
        throw NoSuchMessageException();
    }
    checkErr(status);
    
    sms.state = (State) uState;
    sms.statusReportRequested = (bStatusReport == 'Y');
    sms.rejectDuplicates = (bRejectDuplicates == 'Y');
    sms.messageBody.header = (bHeaderIndicator == 'Y');
    
    tm  dt;
    sb2 year;
    ub1 mon, mday, hour, min, sec;

    OCIDateGetTime(&deliveryTime, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(&deliveryTime, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    sms.deliveryTime = mktime(&dt);

    OCIDateGetTime(&submitTime, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(&submitTime, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    sms.submitTime = mktime(&dt);

    OCIDateGetTime(&waitTime, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(&waitTime, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    sms.waitTime = mktime(&dt);

    OCIDateGetTime(&validTime, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(&validTime, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    sms.validTime = mktime(&dt);
    
    _sms = sms;
}

void Connection::remove(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard  guard(mutex);

    smsId = id;
    checkErr(OCIStmtExecute(svchp, stmtRemove, errhp, (ub4) 1, (ub4) 0,
                            (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                            OCI_DEFAULT));
    if (!smsId) {
        throw NoSuchMessageException();
    }
    checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void Connection::checkConnErr(sword status) 
    throw(ConnectionFailedException)
{
    try 
    {
        checkErr(status);
    } 
    catch (StorageException& exc) 
    {
        throw ConnectionFailedException(exc);
    }
}

void Connection::checkErr(sword status) 
    throw(StorageException)
{
    text        errbuf[1024];
    ub4         buflen, errcode;
    
    switch (status)
    {
    case OCI_SUCCESS:
        return;

    case OCI_SUCCESS_WITH_INFO:
        //throw StoreException(status, "OCI_SUCCESS_WITH_INFO");
        return;
    
    case OCI_NO_DATA:
        strcpy((char *)errbuf, "OCI_NODATA");
        break;
        
    case OCI_NEED_DATA:
        strcpy((char *)errbuf, "OCI_NEED_DATA");
        break;
        
    case OCI_STILL_EXECUTING:
        strcpy((char *)errbuf, "OCI_STILL_EXECUTE");
        break;
    
    case OCI_INVALID_HANDLE:
        strcpy((char *)errbuf, "OCI_INVALID_HANDLE");
        break;
                
    case OCI_ERROR:
        (void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
                            (sb4 *)&errcode, errbuf,
                            (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
        status = errcode;
        break;  

    default:
        strcpy((char *)errbuf, "OCI_UNKNOWN_ERROR");
        break;
    }
    
    __require__(owner);
    owner->killConnection(this);

    throw StorageException((const char *)errbuf, (int)status);
}

/* ------------------------------- Connection -------------------------- */

}}

