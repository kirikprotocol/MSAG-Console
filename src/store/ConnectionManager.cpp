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

void ConnectionPool::loadMaxSize(Manager& config)
{
    try {
        size = (unsigned)config.getInt("MessageStore.Connections.max");
    } catch (ConfigException& exc) {
        size = SMSC_DEFAULT_CONNECTION_POOL_MAX_SIZE;
        log.warn("Max size wasn't specified ! "
                 "Using default: %d", size);
    }
}

void ConnectionPool::loadInitSize(Manager& config)
{
    try {
        count = (unsigned)config.getInt("MessageStore.Connections.init");
    } catch (ConfigException& exc) {
        count = SMSC_DEFAULT_CONNECTION_POOL_INIT_SIZE;
        log.warn("Init size wasn't specified ! "
                 "Using default: %d", count);
    }
}

void ConnectionPool::loadDBInstance(Manager& config)
    throw(ConfigException)
{
    try {
        dbInstance = config.getString("MessageStore.dbInstance");   
    } catch (ConfigException& exc) {
        log.error("DB instance name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserName(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserName = config.getString("MessageStore.dbUserName");   
    } catch (ConfigException& exc) {
        log.error("DB user name wasn't specified !");
        throw;
    }
}

void ConnectionPool::loadDBUserPassword(Manager& config)
    throw(ConfigException)
{
    try {
        dbUserPassword = config.getString("MessageStore.dbUserPassword");   
    } catch (ConfigException& exc) {
        log.error("DB user password wasn't specified !");
        throw;
    }
}

ConnectionPool::ConnectionPool(Manager& config)
    throw(ConfigException, ConnectionFailedException) 
        : log(Logger::getCategory("smsc.store.ConnectionPool"))
{
    
    loadMaxSize(config);
    loadInitSize(config);
    loadDBInstance(config);
    loadDBUserName(config);
    loadDBUserPassword(config);

    if (size < count) 
    {
        size = count;
        log.warn("Specified size less than init size. "
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
        : log(Logger::getCategory("smsc.store.Connection")), 
            owner(pool), envhp(0L), errhp(0L), svchp(0L), srvhp(0L), sesshp(0L) 
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

    // create & prepare statement for select max(id)
    checkConnErr(stmtGetMessagesCount.
                 create(envhp, svchp, errhp, sqlGetMessagesCount));
    checkConnErr(stmtGetMessagesCount.
                 define(1, SQLT_UIN, (dvoid *) &(smsId), 
                        (sb4) sizeof(smsId), errhp));

    // create & prepare statement for insertion
    checkConnErr(stmtStoreInsert.
                 create(envhp, svchp, errhp, sqlStoreInsert));
    checkConnErr(stmtStoreInsert.
                 bind(1 , SQLT_UIN, (dvoid *) &smsId, 
                      (sb4) sizeof(smsId), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(2 , SQLT_UIN, (dvoid *) &(uState), 
                      (sb4) sizeof(uState), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(3 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
                      (sb4) sizeof(sms.messageReference), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(4 , SQLT_UIN, (dvoid *) &(sms.messageIdentifier), 
                      (sb4) sizeof(sms.messageIdentifier), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(5 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
                      (sb4) sizeof(sms.originatingAddress.lenght), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(6 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
                      (sb4) sizeof(sms.originatingAddress.type), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(7 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
                      (sb4) sizeof(sms.originatingAddress.plan), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(8 , SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
                      (sb4) sizeof(sms.originatingAddress.value), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.lenght), 
                      (sb4) sizeof(sms.destinationAddress.lenght), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(10, SQLT_UIN, (dvoid *) &(sms.destinationAddress.type), 
                      (sb4) sizeof(sms.destinationAddress.type), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(11, SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan), 
                      (sb4) sizeof(sms.destinationAddress.plan), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(12, SQLT_STR, (dvoid *) (sms.destinationAddress.value), 
                      (sb4) sizeof(sms.destinationAddress.value), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(13, SQLT_ODT, (dvoid *) &(validTime), 
                      (sb4) sizeof(validTime), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(14, SQLT_ODT, (dvoid *) &(waitTime), 
                      (sb4) sizeof(waitTime), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(15, SQLT_ODT, (dvoid *) &(submitTime), 
                      (sb4) sizeof(submitTime), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(16, SQLT_ODT, (dvoid *) &(deliveryTime), 
                      (sb4) sizeof(deliveryTime), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(17, SQLT_AFC, (dvoid *) &(bStatusReport), 
                      (sb4) sizeof(bStatusReport), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(18, SQLT_AFC, (dvoid *) &(bRejectDuplicates), 
                      (sb4) sizeof(bRejectDuplicates), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(19, SQLT_UIN, (dvoid *) &(sms.priority), 
                      (sb4) sizeof(sms.priority), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(20, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier), 
                      (sb4) sizeof(sms.protocolIdentifier), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(21, SQLT_UIN, (dvoid *) &(sms.failureCause), 
                      (sb4) sizeof(sms.failureCause), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(22, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
                      (sb4) sizeof(sms.messageBody.scheme), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(23, SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
                      (sb4) sizeof(bHeaderIndicator), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(24, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
                      (sb4) sizeof(sms.messageBody.lenght), errhp));
    checkConnErr(stmtStoreInsert.
                 bind(25, SQLT_BIN, (dvoid *) (sms.messageBody.data), 
                      (sb4) sizeof(sms.messageBody.data), errhp));
    
    // create & prepare statement for retriving
    checkConnErr(stmtRetriveAll.
                 create(envhp, svchp, errhp, sqlRetriveAll));
    checkConnErr(stmtRetriveAll.
                 define(1 , SQLT_UIN, (dvoid *) &(uState),
                        (sb4) sizeof(uState), errhp));
    checkConnErr(stmtRetriveAll.
                 define(2 , SQLT_UIN, (dvoid *) &(sms.messageReference),
                        (sb4) sizeof(sms.messageReference), errhp));
    checkConnErr(stmtRetriveAll.
                 define(3 , SQLT_UIN, (dvoid *) &(sms.messageIdentifier),
                        (sb4) sizeof(sms.messageIdentifier), errhp));
    checkConnErr(stmtRetriveAll.
                 define(4 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.lenght),
                        (sb4) sizeof(sms.originatingAddress.lenght), errhp));
    checkConnErr(stmtRetriveAll.
                 define(5 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type),
                        (sb4) sizeof(sms.originatingAddress.type), errhp));
    checkConnErr(stmtRetriveAll.
                 define(6 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan),
                        (sb4) sizeof(sms.originatingAddress.plan), errhp));
    checkConnErr(stmtRetriveAll.
                 define(7 , SQLT_STR, (dvoid *) (sms.originatingAddress.value),
                        (sb4) sizeof(sms.originatingAddress.value), errhp));
    checkConnErr(stmtRetriveAll.
                 define(8 , SQLT_UIN, (dvoid *)&(sms.destinationAddress.lenght),
                        (sb4) sizeof(sms.destinationAddress.lenght), errhp));
    checkConnErr(stmtRetriveAll.
                 define(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.type),
                        (sb4) sizeof(sms.destinationAddress.type), errhp));
    checkConnErr(stmtRetriveAll.
                 define(10, SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan),
                        (sb4) sizeof(sms.destinationAddress.plan), errhp));
    checkConnErr(stmtRetriveAll.
                 define(11, SQLT_STR, (dvoid *) (sms.destinationAddress.value),
                        (sb4) sizeof(sms.destinationAddress.value), errhp));
    checkConnErr(stmtRetriveAll.
                 define(12, SQLT_ODT, (dvoid *) &(validTime),
                        (sb4) sizeof(validTime), errhp));
    checkConnErr(stmtRetriveAll.
                 define(13, SQLT_ODT, (dvoid *) &(waitTime),
                        (sb4) sizeof(waitTime), errhp));
    checkConnErr(stmtRetriveAll.
                 define(14, SQLT_ODT, (dvoid *) &(submitTime),
                        (sb4) sizeof(submitTime), errhp));
    checkConnErr(stmtRetriveAll.
                 define(15, SQLT_ODT, (dvoid *) &(deliveryTime),
                        (sb4) sizeof(deliveryTime), errhp));
    checkConnErr(stmtRetriveAll.
                 define(16, SQLT_AFC, (dvoid *) &(bStatusReport),
                        (sb4) sizeof(bStatusReport), errhp));
    checkConnErr(stmtRetriveAll.
                 define(17, SQLT_AFC, (dvoid *) &(bRejectDuplicates),
                        (sb4) sizeof(bRejectDuplicates), errhp));
    checkConnErr(stmtRetriveAll.
                 define(18, SQLT_UIN, (dvoid *) &(sms.priority),
                        (sb4) sizeof(sms.priority), errhp));
    checkConnErr(stmtRetriveAll.
                 define(19, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier),
                        (sb4) sizeof(sms.protocolIdentifier), errhp));
    checkConnErr(stmtRetriveAll.
                 define(20, SQLT_UIN, (dvoid *) &(sms.failureCause),
                        (sb4) sizeof(sms.failureCause), errhp));
    checkConnErr(stmtRetriveAll.
                 define(21, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
                        (sb4) sizeof(sms.messageBody.scheme), errhp));
    checkConnErr(stmtRetriveAll.
                 define(22, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
                        (sb4) sizeof(bHeaderIndicator), errhp));
    checkConnErr(stmtRetriveAll.
                 define(23, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
                        (sb4) sizeof(sms.messageBody.lenght), errhp));
    checkConnErr(stmtRetriveAll.
                 define(24, SQLT_BIN, (dvoid *) (sms.messageBody.data),
                        (sb4) sizeof(sms.messageBody.data), errhp));
    checkConnErr(stmtRetriveAll.
                 bind(1, SQLT_UIN, (dvoid *) &(smsId),
                      (sb4) sizeof(smsId), errhp));
    
    // create & prepare statement for removing
    checkConnErr(stmtRemove.
                 create(envhp, svchp, errhp, sqlRemove));
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

    checkConnErr(stmtGetMessagesCount.execute(errhp, OCI_DEFAULT));
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
        checkErr(stmtStoreInsert.execute(errhp, OCI_DEFAULT));
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
    if ((status = stmtRetriveAll.execute(errhp, OCI_DEFAULT)) == OCI_NO_DATA)
    {
        NoSuchMessageException  exc;
        log.error("Storage Exception : %s\n", exc.what());
        throw exc;
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
    
    checkErr(stmtRemove.bind(1, SQLT_UIN, (dvoid *) &(id),
                             (sb4) sizeof(id), errhp)); 
    SMSId rows = 0;
    checkErr(stmtRemove.define(1, SQLT_UIN, (dvoid *) &(rows),
                               (sb4) sizeof(rows), errhp));
    
    checkErr(stmtRemove.execute(errhp, OCI_DEFAULT));
    if (!rows) 
    {
        NoSuchMessageException  exc;
        log.error("Storage Exception : %s\n", exc.what());
        throw exc;
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

    StorageException    exc((const char *)errbuf, (int)status);
    log.error("Storage Exception : %s\n", exc.what());
    throw exc;
}

/* ------------------------------- Connection -------------------------- */

}}

