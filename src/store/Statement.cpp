#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ConnectionManager.h"

namespace smsc { namespace store 
{

#define UINT64_SWAP_LE_BE_CONSTANT(val)      ((uint64_t) (\
    (((uint64_t) (val) &                              \
        (uint64_t) (0x00000000000000FFU)) << 56) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x000000000000FF00U)) << 40) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x0000000000FF0000U)) << 24) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x00000000FF000000U)) <<  8) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x000000FF00000000U)) >>  8) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x0000FF0000000000U)) >> 24) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0x00FF000000000000U)) >> 40) |    \
    (((uint64_t) (val) &                              \
        (uint64_t) (0xFF00000000000000U)) >> 56)))

/* ----------------------------- Statetment -------------------------- */

Statement::Statement(Connection* connection, const char* sql) 
    throw(StorageException)
        : owner(connection)
{
    __require__(owner);
    
    envhp = owner->envhp; svchp = owner->svchp; errhp = owner->errhp;
    
    checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmt,
                            OCI_HTYPE_STMT, 0, (dvoid **)0));

    checkErr(OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4) strlen(sql),
                            (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
}

Statement::~Statement()
{
    (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
}

void Statement::bind(ub4 pos, ub2 type, dvoid* placeholder, sb4 size)
    throw(StorageException)
{
    OCIBind *bind;
    checkErr(OCIBindByPos(stmt, &bind, errhp, pos, 
                          placeholder, size, type, 
                          (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                          (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT));
}

void Statement::bind(CONST text* name, sb4 name_len, ub2 type,
                     dvoid* placeholder, sb4 size)
    throw(StorageException)
{
    OCIBind *bind;
    checkErr(OCIBindByName(stmt, &bind, errhp, name, name_len,
                           placeholder, size, type,
                           (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                           (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT));
}

void Statement::define(ub4 pos, ub2 type, 
                       dvoid* placeholder, sb4 size)
    throw(StorageException)
{
    OCIDefine*  define;
    checkErr(OCIDefineByPos(stmt, &define, errhp, pos, 
                            placeholder, size, type, (dvoid *) 0,
                            (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT));
}

sword Statement::execute(ub4 mode, ub4 iters, ub4 rowoff)
{
    return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          mode);
}

sword Statement::fetch()
{
    return OCIStmtFetch(stmt, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
                        (ub4) OCI_DEFAULT);
}

ub4 Statement::getRowsAffectedCount()
{
    ub4 res = 0; 
    if (OCIAttrGet((CONST dvoid *)stmt, OCI_HTYPE_STMT, 
                   &res, NULL, OCI_ATTR_ROW_COUNT, errhp) != OCI_SUCCESS)
    {
        return 0;
    }
    return res;
}

void Statement::checkErr(sword status) 
    throw(StorageException)
{
    __require__(owner);
    owner->checkErr(status);
}

void Statement::convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date)
{
    tm* dt = localtime(sms_date);

    OCIDateSetDate(oci_date, (sb2)(1900+dt->tm_year), 
                   (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
    OCIDateSetTime(oci_date, (ub1)(dt->tm_hour), 
                   (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
}
void Statement::convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date)
{
    tm  dt;
    sb2 year;
    ub1 mon, mday, hour, min, sec;

    OCIDateGetTime(oci_date, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(oci_date, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    *sms_date = mktime(&dt);
}

/* ----------------------------- IdStatetment ------------------------- */  

void IdStatement::setSMSId(const SMSId _smsId)
{
    smsId = UINT64_SWAP_LE_BE_CONSTANT(_smsId);
}
void IdStatement::getSMSId(SMSId &_smsId)
{
    _smsId = UINT64_SWAP_LE_BE_CONSTANT(smsId);
}

/* --------------------------- GetIdStatement ----------------------- */
GetIdStatement::GetIdStatement(Connection* connection, const char* sql)
    throw(StorageException)
        : IdStatement(connection, sql)
{
    define(1, SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* --------------------------- SetIdStatement ----------------------- */
SetIdStatement::SetIdStatement(Connection* connection, const char* sql)
    throw(StorageException)
        : IdStatement(connection, sql)
{
    bind((CONST text*)"ID", (sb4) strlen("ID"), SQLT_BIN, 
         (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* --------------------------- MessageStatetment --------------------- */

void MessageStatement::setSMS(const SMS &_sms)
{
    sms = _sms;
    
    convertDateToOCIDate(&(sms.validTime), &validTime);
    convertDateToOCIDate(&(sms.waitTime), &waitTime);
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    convertDateToOCIDate(&(sms.deliveryTime), &deliveryTime);

    bNeedArchivate = sms.needArchivate ? 'Y':'N';
    bStatusReport = sms.statusReportRequested ? 'Y':'N';
    bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
    uState = (uint8_t) sms.state;
}

void MessageStatement::getSMS(SMS &_sms)
{
    sms.state = (State) uState;
    sms.needArchivate = (bNeedArchivate == 'Y');
    sms.statusReportRequested = (bStatusReport == 'Y');
    sms.messageBody.header = (bHeaderIndicator == 'Y');
    
    convertOCIDateToDate(&deliveryTime, &(sms.deliveryTime));
    convertOCIDateToDate(&submitTime, &(sms.submitTime));
    convertOCIDateToDate(&waitTime, &(sms.waitTime));
    convertOCIDateToDate(&validTime, &(sms.validTime));
    
    _sms = sms;
}

/* --------------------------- StoreStatement ----------------------- */
const char* StoreStatement::sql = (const char*)
"INSERT INTO SMS_MSG VALUES (:ID, :ST, :MR,\
 :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL, :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
 :VALID_TIME, :WAIT_TIME, :SUBMIT_TIME, :DELIVERY_TIME,\
 :SRR, :ARC, :PRI, :PID, :FCS, :DCS, :UDHI, :UDL, :UD)";

StoreStatement::StoreStatement(Connection* connection)
    throw(StorageException)
        : MessageStatement(connection, StoreStatement::sql)
{
    bind(1 , SQLT_BIN, (dvoid *) &smsId, (sb4) sizeof(smsId));
    bind(2 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(3 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
         (sb4) sizeof(sms.messageReference));
    bind(4 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(5 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
         (sb4) sizeof(sms.originatingAddress.type));
    bind(6 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(7 , SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
    bind(8 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.lenght), 
         (sb4) sizeof(sms.destinationAddress.lenght));
    bind(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.type), 
         (sb4) sizeof(sms.destinationAddress.type));
    bind(10, SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan), 
         (sb4) sizeof(sms.destinationAddress.plan));
    bind(11, SQLT_STR, (dvoid *) (sms.destinationAddress.value), 
         (sb4) sizeof(sms.destinationAddress.value));
    bind(12, SQLT_ODT, (dvoid *) &(validTime),
         (sb4) sizeof(validTime));
    bind(13, SQLT_ODT, (dvoid *) &(waitTime), 
         (sb4) sizeof(waitTime));
    bind(14, SQLT_ODT, (dvoid *) &(submitTime), 
         (sb4) sizeof(submitTime));
    bind(15, SQLT_ODT, (dvoid *) &(deliveryTime), 
         (sb4) sizeof(deliveryTime));
    bind(16, SQLT_AFC, (dvoid *) &(bStatusReport), 
         (sb4) sizeof(bStatusReport));
    bind(17, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
         (sb4) sizeof(bNeedArchivate));
    bind(18, SQLT_UIN, (dvoid *) &(sms.priority), 
         (sb4) sizeof(sms.priority));
    bind(19, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier), 
         (sb4) sizeof(sms.protocolIdentifier));
    bind(20, SQLT_UIN, (dvoid *) &(sms.failureCause), 
         (sb4) sizeof(sms.failureCause));
    bind(21, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
         (sb4) sizeof(sms.messageBody.scheme));
    bind(22, SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(23, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
         (sb4) sizeof(sms.messageBody.lenght));
    bind(24, SQLT_BIN, (dvoid *) (sms.messageBody.data), 
         (sb4) sizeof(sms.messageBody.data));
}

/* ----------------------- IsRejectedStatement ---------------------- */
    
const char* IsRejectedStatement::sql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG WHERE ST=:ENROUTE AND MR=:2\
 AND OA_LEN=:3 AND OA_TON=:4 AND OA_NPI=:5 AND OA_VAL=:6\
 AND DA_LEN=:7 AND DA_TON=:8 AND DA_NPI=:9 AND DA_VAL=:10";
    
IsRejectedStatement::IsRejectedStatement(Connection* connection)
    throw(StorageException)
        : MessageStatement(connection, IsRejectedStatement::sql)
{
    bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(2 , SQLT_UIN, (dvoid *) &(sms.messageReference),
         (sb4) sizeof(sms.messageReference));
    bind(3 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.lenght),
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(4 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type),
         (sb4) sizeof(sms.originatingAddress.type));
    bind(5 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan),
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(6 , SQLT_STR, (dvoid *) (sms.originatingAddress.value),
         (sb4) sizeof(sms.originatingAddress.value));
    bind(7 , SQLT_UIN, (dvoid *)&(sms.destinationAddress.lenght),
         (sb4) sizeof(sms.destinationAddress.lenght));
    bind(8 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.type),
         (sb4) sizeof(sms.destinationAddress.type));
    bind(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan),
         (sb4) sizeof(sms.destinationAddress.plan));
    bind(10, SQLT_STR, (dvoid *) (sms.destinationAddress.value),
         (sb4) sizeof(sms.destinationAddress.value));
    
    define(1 , SQLT_UIN, (dvoid *) &(count), (sb4) sizeof(count));
}

bool IsRejectedStatement::isRejected()
{
    return ((count) ? true:false);
}

/* ------------------------ IsTimeCorrectStatement -------------------- */
const char* IsTimeCorrectStatement::sql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG WHERE ST=:ENROUTE AND\
 OA_LEN=:2 AND OA_TON=:3 AND OA_NPI=:4 AND OA_VAL=:5 AND SUBMIT_TIME=:6";

IsTimeCorrectStatement::IsTimeCorrectStatement(Connection* connection)
    throw(StorageException)
        : MessageStatement(connection, IsTimeCorrectStatement::sql)
{
    bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(2 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.lenght),
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(3 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type),
         (sb4) sizeof(sms.originatingAddress.type));
    bind(4 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan),
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(5 , SQLT_STR, (dvoid *) (sms.originatingAddress.value),
         (sb4) sizeof(sms.originatingAddress.value));
    bind(6, SQLT_ODT, (dvoid *) &(submitTime),
         (sb4) sizeof(submitTime));
    
    define(1 , SQLT_UIN, (dvoid *) &(count), (sb4) sizeof(count));
}

bool IsTimeCorrectStatement::isTimeIncorrect()
{
    return ((count) ? true:false);
}

/* --------------------------- RetriveStatement ----------------------- */
const char* RetriveStatement::sql = (const char*)
"SELECT ST, MR, OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON,\
 DA_NPI, DA_VAL, VALID_TIME, WAIT_TIME, SUBMIT_TIME, DELIVERY_TIME,\
 SRR, ARC, PRI, PID, FCS, DCS, UDHI, UDL, UD FROM SMS_MSG WHERE ID=:ID";

RetriveStatement::RetriveStatement(Connection* connection)
    throw(StorageException)
        : MessageStatement(connection, RetriveStatement::sql)
{
    bind(1, SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
    
    define(1 , SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    define(2 , SQLT_UIN, (dvoid *) &(sms.messageReference),
           (sb4) sizeof(sms.messageReference));
    define(3 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.lenght),
           (sb4) sizeof(sms.originatingAddress.lenght));
    define(4 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type),
           (sb4) sizeof(sms.originatingAddress.type));
    define(5 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan),
           (sb4) sizeof(sms.originatingAddress.plan));
    define(6 , SQLT_STR, (dvoid *) (sms.originatingAddress.value),
           (sb4) sizeof(sms.originatingAddress.value));
    define(7 , SQLT_UIN, (dvoid *)&(sms.destinationAddress.lenght),
           (sb4) sizeof(sms.destinationAddress.lenght));
    define(8 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.type),
           (sb4) sizeof(sms.destinationAddress.type));
    define(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan),
           (sb4) sizeof(sms.destinationAddress.plan));
    define(10, SQLT_STR, (dvoid *) (sms.destinationAddress.value),
           (sb4) sizeof(sms.destinationAddress.value));
    define(11, SQLT_ODT, (dvoid *) &(validTime),
           (sb4) sizeof(validTime));
    define(12, SQLT_ODT, (dvoid *) &(waitTime),
           (sb4) sizeof(waitTime));
    define(13, SQLT_ODT, (dvoid *) &(submitTime),
           (sb4) sizeof(submitTime));
    define(14, SQLT_ODT, (dvoid *) &(deliveryTime),
           (sb4) sizeof(deliveryTime));
    define(15, SQLT_AFC, (dvoid *) &(bStatusReport),
           (sb4) sizeof(bStatusReport));
    define(16, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
           (sb4) sizeof(bNeedArchivate));
    define(17, SQLT_UIN, (dvoid *) &(sms.priority),
           (sb4) sizeof(sms.priority));
    define(18, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier),
           (sb4) sizeof(sms.protocolIdentifier));
    define(19, SQLT_UIN, (dvoid *) &(sms.failureCause),
           (sb4) sizeof(sms.failureCause));
    define(20, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
           (sb4) sizeof(sms.messageBody.scheme));
    define(21, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
           (sb4) sizeof(bHeaderIndicator));
    define(22, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
           (sb4) sizeof(sms.messageBody.lenght));
    define(23, SQLT_BIN, (dvoid *) (sms.messageBody.data),
           (sb4) sizeof(sms.messageBody.data));
}

/* --------------------------- ReplaceStatement ----------------------- */        
const char* ReplaceStatement::sql = (const char*)
"UPDATE SMS_MSG SET MR=:MR,\
 DCS=:DCS, UDHI=:UDHI, UDL=:UDL, UD=:UD WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_LEN=:OA_LEN AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";

ReplaceStatement::ReplaceStatement(Connection* connection)
    throw(StorageException)
        : MessageStatement(connection, ReplaceStatement::sql) 
{
    bind(1 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
         (sb4) sizeof(sms.messageReference));
    bind(2 , SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
         (sb4) sizeof(sms.messageBody.scheme));
    bind(3 , SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(4 , SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
         (sb4) sizeof(sms.messageBody.lenght));
    bind(5 , SQLT_BIN, (dvoid *) (sms.messageBody.data), 
         (sb4) sizeof(sms.messageBody.data));
    bind(6 , SQLT_BIN, (dvoid *) &(smsId), 
         (sb4) sizeof(smsId));
    bind(7 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(8 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(9 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
         (sb4) sizeof(sms.originatingAddress.type));
    bind(10, SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(11, SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
}

const char* ReplaceVTStatement::sql = (const char*)
"UPDATE SMS_MSG SET MR=:MR, VALID_TIME=:VT,\
 DCS=:DCS, UDHI=:UDHI, UDL=:UDL, UD=:UD WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_LEN=:OA_LEN AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";

ReplaceVTStatement::ReplaceVTStatement(Connection* connection)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVTStatement::sql) 
{
    bind(1 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
         (sb4) sizeof(sms.messageReference));
    bind(2, SQLT_ODT, (dvoid *) &(validTime),
         (sb4) sizeof(validTime));
    bind(3 , SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
         (sb4) sizeof(sms.messageBody.scheme));
    bind(4 , SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(5 , SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
         (sb4) sizeof(sms.messageBody.lenght));
    bind(6 , SQLT_BIN, (dvoid *) (sms.messageBody.data), 
         (sb4) sizeof(sms.messageBody.data));
    bind(7 , SQLT_BIN, (dvoid *) &(smsId), 
         (sb4) sizeof(smsId));
    bind(8 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(9 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(10, SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
         (sb4) sizeof(sms.originatingAddress.type));
    bind(11, SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(12, SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
}

const char* ReplaceWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET MR=:MR, WAIT_TIME=:WT,\
 DCS=:DCS, UDHI=:UDHI, UDL=:UDL, UD=:UD WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_LEN=:OA_LEN AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";

ReplaceWTStatement::ReplaceWTStatement(Connection* connection)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceWTStatement::sql) 
{
    bind(1 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
         (sb4) sizeof(sms.messageReference));
    bind(2, SQLT_ODT, (dvoid *) &(waitTime),
         (sb4) sizeof(waitTime));
    bind(3 , SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
         (sb4) sizeof(sms.messageBody.scheme));
    bind(4 , SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(5 , SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
         (sb4) sizeof(sms.messageBody.lenght));
    bind(6 , SQLT_BIN, (dvoid *) (sms.messageBody.data), 
         (sb4) sizeof(sms.messageBody.data));
    bind(7 , SQLT_BIN, (dvoid *) &(smsId), 
         (sb4) sizeof(smsId));
    bind(8 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(9 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(10, SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
         (sb4) sizeof(sms.originatingAddress.type));
    bind(11, SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(12, SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
}

const char* ReplaceVWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET MR=:MR, VALID_TIME=:VT, WAIT_TIME=:WT,\
 DCS=:DCS, UDHI=:UDHI, UDL=:UDL, UD=:UD WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_LEN=:OA_LEN AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";

ReplaceVWTStatement::ReplaceVWTStatement(Connection* connection)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVWTStatement::sql) 
{
    bind(1 , SQLT_UIN, (dvoid *) &(sms.messageReference), 
         (sb4) sizeof(sms.messageReference));
    bind(2, SQLT_ODT, (dvoid *) &(validTime),
         (sb4) sizeof(validTime));
    bind(3, SQLT_ODT, (dvoid *) &(waitTime),
         (sb4) sizeof(waitTime));
    bind(4 , SQLT_UIN, (dvoid *) &(sms.messageBody.scheme), 
         (sb4) sizeof(sms.messageBody.scheme));
    bind(5 , SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(6 , SQLT_UIN, (dvoid *) &(sms.messageBody.lenght), 
         (sb4) sizeof(sms.messageBody.lenght));
    bind(7 , SQLT_BIN, (dvoid *) (sms.messageBody.data), 
         (sb4) sizeof(sms.messageBody.data));
    bind(8 , SQLT_BIN, (dvoid *) &(smsId), 
         (sb4) sizeof(smsId));
    bind(9 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind(10, SQLT_UIN, (dvoid *) &(sms.originatingAddress.lenght), 
         (sb4) sizeof(sms.originatingAddress.lenght));
    bind(11, SQLT_UIN, (dvoid *) &(sms.originatingAddress.type), 
         (sb4) sizeof(sms.originatingAddress.type));
    bind(12, SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan), 
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(13, SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
}

/* --------------------------- RemoveStatement ----------------------- */
const char* RemoveStatement::sql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID AND ST=:ENROUTE";

RemoveStatement::RemoveStatement(Connection* connection)
    throw(StorageException)
        : IdStatement(connection, RemoveStatement::sql)
{
    bind(1, SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
    bind(2 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

/* --------------------------- UpdateStatements ----------------------- */
const char* StateUpdateStatement::sql = (const char*)
"UPDATE SMS_MSG SET ST=:ST WHERE ID=:ID AND ST=:ENROUTE";

StateUpdateStatement::StateUpdateStatement(Connection* connection)
    throw(StorageException)
        : IdStatement(connection, StateUpdateStatement::sql)
{
    bind(1 , SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    bind(2 , SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
    bind(3 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

const char* StateDateUpdateStatement::sql = (const char*)
"UPDATE SMS_MSG SET ST=:ST, DELIVERY_TIME=:DT WHERE ID=:ID AND ST=:ENROUTE";

StateDateUpdateStatement::StateDateUpdateStatement(Connection* connection)
        throw(StorageException)
            : StateUpdateStatement(connection, StateDateUpdateStatement::sql)
{
    bind(1 , SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    bind(2 , SQLT_ODT, (dvoid *) &(operationDate), (sb4) sizeof(operationDate));
    bind(3 , SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
    bind(4 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

const char* StateDateFcsUpdateStatement::sql = (const char*)
"UPDATE SMS_MSG SET ST=:ST, DELIVERY_TIME=:DT, FCS=:FC WHERE ID=:ID AND ST=:ENROUTE";

StateDateFcsUpdateStatement::StateDateFcsUpdateStatement(Connection* connection)
        throw(StorageException)
            : StateDateUpdateStatement(connection, StateDateFcsUpdateStatement::sql)
{
    bind(1 , SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    bind(2 , SQLT_ODT, (dvoid *) &(operationDate), (sb4) sizeof(operationDate));
    bind(3 , SQLT_UIN, (dvoid *) &(fcs), (sb4) sizeof(fcs));
    bind(4 , SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
    bind(5 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

}}

