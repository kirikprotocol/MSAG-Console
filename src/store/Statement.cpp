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

Statement::Statement(Connection* connection, const char* sql, bool assign) 
    throw(StorageException)
        : owner(connection)
{
    __require__(owner);
    
    envhp = owner->envhp; svchp = owner->svchp; errhp = owner->errhp;
    
    check(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmt,
                         OCI_HTYPE_STMT, 0, (dvoid **)0));
    check(OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4) strlen(sql),
                         (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    
    if (assign) owner->assign(this);
}

Statement::~Statement()
{
    (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
}

void Statement::bind(ub4 pos, ub2 type, 
                     dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    OCIBind *bind;
    check(OCIBindByPos(stmt, &bind, errhp, pos, 
                       placeholder, size, type, indp,
                       (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                       (ub4) OCI_DEFAULT));
}

void Statement::bind(CONST text* name, sb4 name_len, ub2 type,
                     dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    OCIBind *bind;
    check(OCIBindByName(stmt, &bind, errhp, name, name_len,
                        placeholder, size, type, indp,
                        (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                        (ub4) OCI_DEFAULT));
}

void Statement::define(ub4 pos, ub2 type, 
                       dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    OCIDefine*  define;
    check(OCIDefineByPos(stmt, &define, errhp, pos, 
                         placeholder, size, type, indp,
                         (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT));
}

sword Statement::execute(ub4 mode, ub4 iters, ub4 rowoff)
{
    return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, 
                          (OCISnapshot *) NULL, mode);
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

void Statement::convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date)
{
    tm dt; localtime_r(sms_date, &dt);

    OCIDateSetDate(oci_date, (sb2)(1900+dt.tm_year), 
                   (ub1)(1+dt.tm_mon), (ub1)(dt.tm_mday));
    OCIDateSetTime(oci_date, (ub1)(dt.tm_hour), 
                   (ub1)(dt.tm_min), (ub1)(dt.tm_sec));
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
void Statement::check(sword status) 
    throw(StorageException) 
{
    __require__(owner);
    owner->check(status);
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
GetIdStatement::GetIdStatement(Connection* connection, const char* sql, 
                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    define(1, SQLT_BIN, (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* --------------------------- SetIdStatement ----------------------- */
SetIdStatement::SetIdStatement(Connection* connection, const char* sql,
                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    bind((CONST text*)"ID", (sb4) strlen("ID"), SQLT_BIN, 
         (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* ----------------------- NeedRejectStatement ---------------------- */
const char* NeedRejectStatement::sql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG WHERE ST=:ENROUTE AND MR=:MR\
 AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL\
 AND DA_TON=:OA_TON AND DA_NPI=:DA_NPI AND DA_VAL=:DA_VAL";

NeedRejectStatement::NeedRejectStatement(Connection* connection, 
                                         bool assign)
    throw(StorageException)
        : Statement(connection, NeedRejectStatement::sql, assign)
{
    bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    define(1 , SQLT_UIN, (dvoid *) &(count), (sb4) sizeof(count));
}

void NeedRejectStatement::bindMr(dvoid* mr, sb4 size)
    throw(StorageException)
{
    bind(2 , SQLT_UIN, mr, size);
}
void NeedRejectStatement::bindOriginatingAddress(Address& oa)
    throw(StorageException)
{
    bind(3 , SQLT_UIN, (dvoid *)&(oa.type), (sb4) sizeof(oa.type)); 
    bind(4 , SQLT_UIN, (dvoid *)&(oa.plan), (sb4) sizeof(oa.plan));
    bind(5 , SQLT_STR, (dvoid *) (oa.value), (sb4) sizeof(oa.value));
}
void NeedRejectStatement::bindDestinationAddress(Address& da)
    throw(StorageException)
{
    bind(6 , SQLT_UIN, (dvoid *)&(da.type), (sb4) sizeof(da.type)); 
    bind(7 , SQLT_UIN, (dvoid *)&(da.plan), (sb4) sizeof(da.plan));
    bind(8 , SQLT_STR, (dvoid *) (da.value), (sb4) sizeof(da.value));
}

bool NeedRejectStatement::needReject()
{
    return ((count) ? true:false);
}

/* ----------------------- NeedOverwriteStatement ---------------------- */
const char* NeedOverwriteStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE ST=:ENROUTE\
 AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL\
 AND DA_TON=:OA_TON AND DA_NPI=:DA_NPI AND DA_VAL=:DA_VAL\
 AND SVC_TYPE IS NULL";
NeedOverwriteStatement::NeedOverwriteStatement(Connection* connection, 
                                               bool assign)
    throw(StorageException)
        : IdStatement(connection, NeedOverwriteStatement::sql, assign)
{
    bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    define(1, SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
NeedOverwriteStatement::NeedOverwriteStatement(Connection* connection,
                                               const char* sql,
                                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    bind(1 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    define(1, SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}

void NeedOverwriteStatement::bindOriginatingAddress(Address& oa)
    throw(StorageException)
{
    bind(2 , SQLT_UIN, (dvoid *)&(oa.type), (sb4) sizeof(oa.type)); 
    bind(3 , SQLT_UIN, (dvoid *)&(oa.plan), (sb4) sizeof(oa.plan));
    bind(4 , SQLT_STR, (dvoid *) (oa.value), (sb4) sizeof(oa.value));
}
void NeedOverwriteStatement::bindDestinationAddress(Address& da)
    throw(StorageException)
{
    bind(5 , SQLT_UIN, (dvoid *)&(da.type), (sb4) sizeof(da.type)); 
    bind(6 , SQLT_UIN, (dvoid *)&(da.plan), (sb4) sizeof(da.plan));
    bind(7 , SQLT_STR, (dvoid *) (da.value), (sb4) sizeof(da.value));
}

void NeedOverwriteStatement::getId(SMSId& id)
    throw(StorageException)
{
    getSMSId(id);
}

const char* NeedOverwriteSvcStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE ST=:ENROUTE\
 AND OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL\
 AND DA_TON=:OA_TON AND DA_NPI=:DA_NPI AND DA_VAL=:DA_VAL\
 AND SVC_TYPE=:SVC_TYPE";
NeedOverwriteSvcStatement::NeedOverwriteSvcStatement(Connection* connection, 
                                                     bool assign=true)
    throw(StorageException)
        : NeedOverwriteStatement(connection, 
                                 NeedOverwriteSvcStatement::sql, assign)
{
}
void NeedOverwriteSvcStatement::bindEServiceType(dvoid* type, sb4 size)
    throw(StorageException)
{
    bind(8 , SQLT_STR, type, size);
}

/* ------------------------- OverwriteStatement ---------------------- */
const char* OverwriteStatement::sql = (const char*)
"UPDATE SMS_MSG SET MR=:MR,\
 SRC_MSC=:SRC_MSC, SRC_IMSI=:SRC_IMSI, SRC_SME_N=:SRC_SME_N,\
 DST_MSC=NULL, DST_IMSI=NULL, DST_SME_N=NULL,\
 WAIT_TIME=:WAIT_TIME, VALID_TIME=:VALID_TIME,\
 SUBMIT_TIME=:SUBMIT_TIME, ATTEMPTS=0, LAST_RESULT=0,\
 LAST_TRY_TIME=NULL, NEXT_TRY_TIME=NULL, SVC_TYPE=:SVC,\
 DR=:DR, ARC=:ARC, PRI=:PRI, PID=:PID,\
 UDHI=:UDHI, DCS=:DCS, UDL=:UDL, UD=:UD\
 WHERE ID=:ID";
OverwriteStatement::OverwriteStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, OverwriteStatement::sql, assign)
{
}
void OverwriteStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
void OverwriteStatement::bindSms(SMS& sms)
    throw(StorageException)
{
    bind(1 , SQLT_UIN, (dvoid *)&(sms.messageReference),
         (sb4) sizeof(sms.messageReference));
    bind(2 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
         (sb4) sizeof(sms.originatingDescriptor.msc));
    bind(3 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
         (sb4) sizeof(sms.originatingDescriptor.imsi));
    bind(4 , SQLT_UIN, (dvoid *)&(sms.originatingDescriptor.sme),
         (sb4) sizeof(sms.originatingDescriptor.sme));
    if (sms.waitTime) 
    {
        convertDateToOCIDate(&(sms.waitTime), &waitTime);
        indWaitTime = OCI_IND_NOTNULL;
    } 
    else indWaitTime = OCI_IND_NULL;
    bind(5 , SQLT_ODT, (dvoid *) &(waitTime), 
         (sb4) sizeof(waitTime), &indWaitTime);
    
    convertDateToOCIDate(&(sms.validTime), &validTime);
    bind(6 , SQLT_ODT, (dvoid *) &(validTime), 
         (sb4) sizeof(validTime));
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    bind(7 , SQLT_ODT, (dvoid *) &(submitTime), 
         (sb4) sizeof(submitTime));
    
    indSvcType = strlen(sms.eServiceType) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(8 , SQLT_STR, (dvoid *) (sms.eServiceType),
         (sb4) sizeof(sms.eServiceType), &indSvcType);
    bind(9 , SQLT_UIN, (dvoid *)&(sms.deliveryReport),
         (sb4) sizeof(sms.deliveryReport));
    bNeedArchivate = sms.needArchivate ? 'Y':'N';
    bind(10, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
         (sb4) sizeof(bNeedArchivate));
    bind(11, SQLT_UIN, (dvoid *)&(sms.priority),
         (sb4) sizeof(sms.priority));
    bind(12, SQLT_UIN, (dvoid *)&(sms.protocolIdentifier),
         (sb4) sizeof(sms.protocolIdentifier));
    bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
    bind(13, SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(14, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
         (sb4) sizeof(sms.messageBody.scheme));
    bind(15, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
         (sb4) sizeof(sms.messageBody.lenght));
    bind(16, SQLT_BIN, (dvoid *) (sms.messageBody.data),
         (sb4) sizeof(sms.messageBody.data));
}

/* --------------------------- StoreStatement ----------------------- */
const char* StoreStatement::sql = (const char*)
"INSERT INTO SMS_MSG (ID, ST, MR,\
 OA_TON, OA_NPI, OA_VAL, SRC_MSC, SRC_IMSI, SRC_SME_N,\
 DA_TON, DA_NPI, DA_VAL,\
 WAIT_TIME, VALID_TIME, SUBMIT_TIME, ATTEMPTS, LAST_RESULT,\
 LAST_TRY_TIME, NEXT_TRY_TIME, SVC_TYPE, DR, ARC, PRI, PID,\
 UDHI, DCS, UDL, UD)\
 VALUES (:ID, :ST, :MR,\
 :OA_TON, :OA_NPI, :OA_VAL, :SRC_MSC, :SRC_IMSI, :SRC_SME_N,\
 :DA_TON, :DA_NPI, :DA_VAL,\
 :WAIT_TIME, :VALID_TIME, :SUBMIT_TIME, 0, 0, NULL, NULL,\
 :SVC, :DR, :ARC, :PRI, :PID, :UDHI, :DCS, :UDL, :UD)";
StoreStatement::StoreStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, StoreStatement::sql, assign)
{
    bind(2 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}
void StoreStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind(1 , SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
void StoreStatement::bindSms(SMS& sms)
    throw(StorageException)
{
    bind(3 , SQLT_UIN, (dvoid *)&(sms.messageReference),
         (sb4) sizeof(sms.messageReference));
    bind(4 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.type),
         (sb4) sizeof(sms.originatingAddress.type)); 
    bind(5 , SQLT_UIN, (dvoid *)&(sms.originatingAddress.plan),
         (sb4) sizeof(sms.originatingAddress.plan));
    bind(6 , SQLT_STR, (dvoid *) (sms.originatingAddress.value), 
         (sb4) sizeof(sms.originatingAddress.value));
    bind(7 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
         (sb4) sizeof(sms.originatingDescriptor.msc));
    bind(8 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
         (sb4) sizeof(sms.originatingDescriptor.imsi));
    bind(9 , SQLT_UIN, (dvoid *)&(sms.originatingDescriptor.sme),
         (sb4) sizeof(sms.originatingDescriptor.sme));
    bind(10, SQLT_UIN, (dvoid *)&(sms.destinationAddress.type),
         (sb4) sizeof(sms.destinationAddress.type)); 
    bind(11, SQLT_UIN, (dvoid *)&(sms.destinationAddress.plan),
         (sb4) sizeof(sms.destinationAddress.plan));
    bind(12, SQLT_STR, (dvoid *) (sms.destinationAddress.value),
         (sb4) sizeof(sms.destinationAddress.value));
    
    if (sms.waitTime) 
    {
        convertDateToOCIDate(&(sms.waitTime), &waitTime);
        indWaitTime = OCI_IND_NOTNULL;
    } 
    else indWaitTime = OCI_IND_NULL;
    bind(13, SQLT_ODT, (dvoid *) &(waitTime), 
         (sb4) sizeof(waitTime), &indWaitTime);
    
    convertDateToOCIDate(&(sms.validTime), &validTime);
    bind(14, SQLT_ODT, (dvoid *) &(validTime), 
         (sb4) sizeof(validTime));
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    bind(15, SQLT_ODT, (dvoid *) &(submitTime), 
         (sb4) sizeof(submitTime));
    
    indSvcType = strlen(sms.eServiceType) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(16, SQLT_STR, (dvoid *) (sms.eServiceType),
         (sb4) sizeof(sms.eServiceType), &indSvcType);
    
    bind(17, SQLT_UIN, (dvoid *)&(sms.deliveryReport),
         (sb4) sizeof(sms.deliveryReport));
    bNeedArchivate = sms.needArchivate ? 'Y':'N';
    bind(18, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
         (sb4) sizeof(bNeedArchivate));
    bind(19, SQLT_UIN, (dvoid *)&(sms.priority),
         (sb4) sizeof(sms.priority));
    bind(20, SQLT_UIN, (dvoid *)&(sms.protocolIdentifier),
         (sb4) sizeof(sms.protocolIdentifier));
    bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
    bind(21, SQLT_AFC, (dvoid *) &(bHeaderIndicator), 
         (sb4) sizeof(bHeaderIndicator));
    bind(22, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
         (sb4) sizeof(sms.messageBody.scheme));
    bind(23, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
         (sb4) sizeof(sms.messageBody.lenght));
    bind(24, SQLT_BIN, (dvoid *) (sms.messageBody.data),
         (sb4) sizeof(sms.messageBody.data));
}

/* --------------------------- RetriveStatement ----------------------- */
const char* RetriveStatement::sql = (const char*)
"SELECT ST, MR,\
 OA_TON, OA_NPI, OA_VAL, SRC_MSC, SRC_IMSI, SRC_SME_N,\
 DA_TON, DA_NPI, DA_VAL, DST_MSC, DST_IMSI, DST_SME_N,\
 WAIT_TIME, VALID_TIME, SUBMIT_TIME,\
 ATTEMPTS, LAST_RESULT, LAST_TRY_TIME, NEXT_TRY_TIME,\
 SVC_TYPE, DR, ARC, PRI, PID,\
 UDHI, DCS, UDL, UD FROM SMS_MSG WHERE ID=:ID";

RetriveStatement::RetriveStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, RetriveStatement::sql, assign)
{
}

void RetriveStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind(1 , SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}

void RetriveStatement::defineSms(SMS& sms)
    throw(StorageException)
{
    define(1 , SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    define(2 , SQLT_UIN, (dvoid *) &(sms.messageReference),
           (sb4) sizeof(sms.messageReference));
    define(3 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.type),
           (sb4) sizeof(sms.originatingAddress.type));
    define(4 , SQLT_UIN, (dvoid *) &(sms.originatingAddress.plan),
           (sb4) sizeof(sms.originatingAddress.plan));
    define(5 , SQLT_STR, (dvoid *) (sms.originatingAddress.value),
           (sb4) sizeof(sms.originatingAddress.value),
           (dvoid *)&(indOA));
    define(6 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
           (sb4) sizeof(sms.originatingDescriptor.msc),
           (dvoid *)&(indSrcMsc));
    define(7 , SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
           (sb4) sizeof(sms.originatingDescriptor.imsi),
           (dvoid *)&(indSrcImsi));
    define(8 , SQLT_UIN, (dvoid *) &(sms.originatingDescriptor.sme),
           (sb4) sizeof(sms.originatingDescriptor.sme),
           (dvoid *)&(indSrcSme));
    define(9 , SQLT_UIN, (dvoid *) &(sms.destinationAddress.type),
           (sb4) sizeof(sms.destinationAddress.type));
    define(10, SQLT_UIN, (dvoid *) &(sms.destinationAddress.plan),
           (sb4) sizeof(sms.destinationAddress.plan));
    define(11, SQLT_STR, (dvoid *) (sms.destinationAddress.value),
           (sb4) sizeof(sms.destinationAddress.value),
           (dvoid *)&(indDA));
    define(12, SQLT_STR, (dvoid *) (sms.destinationDescriptor.msc),
           (sb4) sizeof(sms.destinationDescriptor.msc), 
           (dvoid *)&indDstMsc);
    define(13, SQLT_STR, (dvoid *) (sms.destinationDescriptor.imsi),
           (sb4) sizeof(sms.destinationDescriptor.imsi),
           (dvoid *)&(indDstImsi));
    define(14, SQLT_UIN, (dvoid *) &(sms.destinationDescriptor.sme),
           (sb4) sizeof(sms.destinationDescriptor.sme),
           (dvoid *)&(indDstSme));
    define(15, SQLT_ODT, (dvoid *) &(waitTime),
           (sb4) sizeof(waitTime),
           (dvoid *)&(indWaitTime));
    define(16, SQLT_ODT, (dvoid *) &(validTime),
           (sb4) sizeof(validTime));
    define(17, SQLT_ODT, (dvoid *) &(submitTime),
           (sb4) sizeof(submitTime));
    define(18, SQLT_UIN, (dvoid *) &(sms.attempts),
           (sb4) sizeof(sms.attempts));
    define(19, SQLT_UIN, (dvoid *) &(sms.failureCause),
           (sb4) sizeof(sms.failureCause));
    define(20, SQLT_ODT, (dvoid *) &(lastTime),
           (sb4) sizeof(lastTime),
           (dvoid *)&(indLastTime));
    define(21, SQLT_ODT, (dvoid *) &(nextTime),
           (sb4) sizeof(nextTime),
           (dvoid *)&(indNextTime));
    define(22, SQLT_STR, (dvoid *) &(sms.eServiceType),
           (sb4) sizeof(sms.eServiceType),
           (dvoid *) &(indSvc));
    define(23, SQLT_UIN, (dvoid *) &(sms.deliveryReport),
           (sb4) sizeof(sms.deliveryReport));
    define(24, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
           (sb4) sizeof(bNeedArchivate));
    define(25, SQLT_UIN, (dvoid *) &(sms.priority),
           (sb4) sizeof(sms.priority));
    define(26, SQLT_UIN, (dvoid *) &(sms.protocolIdentifier),
           (sb4) sizeof(sms.protocolIdentifier));
    define(27, SQLT_AFC, (dvoid *) &(bHeaderIndicator),
           (sb4) sizeof(bHeaderIndicator));
    define(28, SQLT_UIN, (dvoid *) &(sms.messageBody.scheme),
           (sb4) sizeof(sms.messageBody.scheme));
    define(29, SQLT_UIN, (dvoid *) &(sms.messageBody.lenght),
           (sb4) sizeof(sms.messageBody.lenght));
    define(30, SQLT_BIN, (dvoid *) (sms.messageBody.data),
           (sb4) sizeof(sms.messageBody.data));
}

void RetriveStatement::getSms(SMS& sms)
{
    sms.state = (State) uState;
    sms.needArchivate = (bNeedArchivate == 'Y');
    sms.messageBody.header = (bHeaderIndicator == 'Y');
    
    if (indOA == OCI_IND_NOTNULL) {
        sms.originatingAddress.lenght 
            = strlen(sms.originatingAddress.value);
    } else {
        sms.originatingAddress.value[0] = '\0';
        sms.originatingAddress.lenght = 0;
    }
    if (indDA == OCI_IND_NOTNULL) {
        sms.destinationAddress.lenght 
            = strlen(sms.destinationAddress.value);
    } else {
        sms.destinationAddress.value[0] = '\0';
        sms.destinationAddress.lenght = 0;
    }
    if (indSrcImsi == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.imsiLenght 
            = strlen(sms.originatingDescriptor.imsi);
    } else {
        sms.originatingDescriptor.imsi[0] = '\0';
        sms.originatingDescriptor.imsiLenght = 0;
    }
    if (indSrcMsc == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.mscLenght 
            = strlen(sms.originatingDescriptor.msc);
    } else {
        sms.originatingDescriptor.msc[0] = '\0';
        sms.originatingDescriptor.mscLenght = 0;
    }
    if (indDstImsi == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.imsiLenght 
            = strlen(sms.destinationDescriptor.imsi);
    } else {
        sms.destinationDescriptor.imsi[0] = '\0';
        sms.destinationDescriptor.imsiLenght = 0;
    }
    if (indDstMsc == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.mscLenght 
            = strlen(sms.destinationDescriptor.msc);
    } else {
        sms.destinationDescriptor.msc[0] = '\0';
        sms.destinationDescriptor.mscLenght = 0;
    }
    
    if (indSrcSme != OCI_IND_NOTNULL) 
        sms.originatingDescriptor.sme = 0;
    if (indDstSme != OCI_IND_NOTNULL) 
        sms.destinationDescriptor.sme = 0;
    if (indSvc != OCI_IND_NOTNULL) 
        sms.eServiceType[0] = '\0';

    if (indLastTime != OCI_IND_NOTNULL) sms.lastTime = 0;
    else convertOCIDateToDate(&lastTime, &(sms.lastTime));
    if (indNextTime != OCI_IND_NOTNULL) sms.nextTime = 0;
    else convertOCIDateToDate(&nextTime, &(sms.nextTime));
    if (indWaitTime != OCI_IND_NOTNULL) sms.waitTime = 0;
    else convertOCIDateToDate(&waitTime, &(sms.waitTime));
    
    convertOCIDateToDate(&submitTime, &(sms.submitTime));
    convertOCIDateToDate(&validTime, &(sms.validTime));
}

/* --------------------------- DestroyStatement ----------------------- */
const char* DestroyStatement::sql = (const char*)
"DELETE FROM SMS_MSG WHERE ID=:ID";

DestroyStatement::DestroyStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, DestroyStatement::sql, assign)
{
}
void DestroyStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind(1 , SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}

/* --------------------------- ReplaceStatement ----------------------- */        
const char* ReplaceStatement::sql = (const char*)
"UPDATE SMS_MSG SET DR=:DR, UDL=:UDL, UD=:UD\
 WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";

ReplaceStatement::ReplaceStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ReplaceStatement::sql, assign) 
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char), 
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}
ReplaceStatement::ReplaceStatement(Connection* connection, const char* sql,
                                   bool assign)
    throw(StorageException) 
        : IdStatement(connection, sql, assign) 
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char), 
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}

void ReplaceStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
void ReplaceStatement::bindOriginatingAddress(Address& oa)
    throw(StorageException)
{
    bind((CONST text *)"OA_TON", (sb4) 6*sizeof(char),
         SQLT_UIN, (dvoid *)&(oa.type), (sb4) sizeof(oa.type));
    bind((CONST text *)"OA_NPI", (sb4) 6*sizeof(char),
         SQLT_UIN, (dvoid *)&(oa.plan), (sb4) sizeof(oa.plan));
    bind((CONST text *)"OA_VAL", (sb4) 6*sizeof(char),
         SQLT_STR, (dvoid *) (oa.value), (sb4) sizeof(oa.value));
}
void ReplaceStatement::bindBody(Body& body)
    throw(StorageException)
{
    bind((CONST text *)"UDL", (sb4) 3*sizeof(char), 
         SQLT_UIN, (dvoid *) &(body.lenght), (sb4) sizeof(body.lenght));
    bind((CONST text *)"UD", (sb4) 2*sizeof(char),
         SQLT_BIN, (dvoid *) (body.data), (sb4) sizeof(body.data));
}
void ReplaceStatement::bindDeliveryReport(dvoid* dr, sb4 size)
    throw(StorageException)
{
    bind((CONST text *)"DR", (sb4) 2*sizeof(char), SQLT_UIN, dr, size);
}
void ReplaceStatement::bindValidTime(time_t validTime)
    throw(StorageException)
{
    convertDateToOCIDate(&(validTime), &vTime);
    bind((CONST text *)"VT", (sb4) 2*sizeof(char), 
         SQLT_ODT, (dvoid *) &(vTime), (sb4) sizeof(vTime));
}
void ReplaceStatement::bindWaitTime(time_t waitTime)
    throw(StorageException)
{
    convertDateToOCIDate(&(waitTime), &wTime);
    bind((CONST text *)"WT", (sb4) 2*sizeof(char), 
         SQLT_ODT, (dvoid *) &(wTime), (sb4) sizeof(wTime));
}

const char* ReplaceVTStatement::sql = (const char*)
"UPDATE SMS_MSG SET DR=:DR, UDL=:UDL, UD=:UD,\
 VALID_TIME=:VT\
 WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";
ReplaceVTStatement::ReplaceVTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVTStatement::sql, assign) {}

const char* ReplaceWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET DR=:DR, UDL=:UDL, UD=:UD,\
 WAIT_TIME=:WT\
 WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";
ReplaceWTStatement::ReplaceWTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceWTStatement::sql, assign) {}

const char* ReplaceVWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET DR=:DR, UDL=:UDL, UD=:UD,\
 VALID_TIME=:VT, WAIT_TIME=:WT\
 WHERE ID=:ID AND ST=:ENROUTE AND\
 OA_TON=:OA_TON AND OA_NPI=:OA_NPI AND OA_VAL=:OA_VAL";
ReplaceVWTStatement::ReplaceVWTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVWTStatement::sql, assign) {}

/* ------------------------ ChangeStateStatements ----------------------- */
const char* ToEnrouteStatement::sql = (const char*)
"UPDATE SMS_MSG SET ATTEMPTS=ATTEMPTS+1, LAST_TRY_TIME=:CT,\
 NEXT_TRY_TIME=:NT, LAST_RESULT=:FCS,\
 DST_MSC=:MSC, DST_IMSI=:IMSI, DST_SME_N=:SME_N \
 WHERE ID=:ID AND ST=:ENROUTE";
ToEnrouteStatement::ToEnrouteStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ToEnrouteStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}
void ToEnrouteStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
void ToEnrouteStatement::bindNextTime(time_t nextTryTime)
    throw(StorageException)
{
    time_t cTime = time(NULL);
    convertDateToOCIDate(&(cTime), &currTime);
    bind(1, SQLT_ODT, (dvoid *) &(currTime), (sb4) sizeof(currTime));
    convertDateToOCIDate(&(nextTryTime), &nextTime);
    bind(2, SQLT_ODT, (dvoid *) &(nextTime), (sb4) sizeof(nextTime));
}
void ToEnrouteStatement::bindFailureCause(dvoid* cause, sb4 size)
    throw(StorageException)
{
    bind(3, SQLT_UIN, cause, size);
}
void ToEnrouteStatement::bindDestinationDescriptor(Descriptor& dst)
    throw(StorageException)
{
    indDstMsc = (!dst.mscLenght || !strlen(dst.msc)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;
    indDstImsi = (!dst.imsiLenght || !strlen(dst.imsi)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;

    bind(4 , SQLT_STR, (dvoid *) (dst.msc), 
         (sb4) sizeof(dst.msc), &indDstMsc);
    bind(5 , SQLT_STR, (dvoid *) (dst.imsi),
         (sb4) sizeof(dst.imsi), &indDstImsi);
    bind(6 , SQLT_UIN, (dvoid *)&(dst.sme),
         (sb4) sizeof(dst.sme));
}

const char* ToDeliveredStatement::sql = (const char*)
"UPDATE SMS_MSG SET ATTEMPTS=ATTEMPTS+1, LAST_TRY_TIME=:CT,\
 NEXT_TRY_TIME=NULL, LAST_RESULT=0,\
 ST=:DELIVERED, DST_MSC=:MSC, DST_IMSI=:IMSI, DST_SME_N=:SME_N \
 WHERE ID=:ID AND ST=:ENROUTE";
ToDeliveredStatement::ToDeliveredStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ToDeliveredStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind((CONST text *)"DELIVERED", (sb4) 9*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_DELIVERED_STATE),
         (sb4) sizeof(SMSC_BYTE_DELIVERED_STATE));
}
void ToDeliveredStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
    
    time_t cTime = time(NULL);
    convertDateToOCIDate(&(cTime), &currTime);
    bind(1, SQLT_ODT, (dvoid *) &(currTime), (sb4) sizeof(currTime));
}
void ToDeliveredStatement::bindDestinationDescriptor(Descriptor& dst)
    throw(StorageException)
{
    indDstMsc = (!dst.mscLenght || !strlen(dst.msc)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;
    indDstImsi = (!dst.imsiLenght || !strlen(dst.imsi)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;

    bind(3 , SQLT_STR, (dvoid *) (dst.msc), 
         (sb4) sizeof(dst.msc), &indDstMsc);
    bind(4 , SQLT_STR, (dvoid *) (dst.imsi),
         (sb4) sizeof(dst.imsi), &indDstImsi);
    bind(5 , SQLT_UIN, (dvoid *)&(dst.sme),
         (sb4) sizeof(dst.sme));
}

const char* ToUndeliverableStatement::sql = (const char*)
"UPDATE SMS_MSG SET ATTEMPTS=ATTEMPTS+1, LAST_TRY_TIME=:CT,\
 NEXT_TRY_TIME=NULL, LAST_RESULT=:FCS,\
 ST=:UNDELIVERABLE, DST_MSC=:MSC, DST_IMSI=:IMSI, DST_SME_N=:SME_N \
 WHERE ID=:ID AND ST=:ENROUTE";
ToUndeliverableStatement::ToUndeliverableStatement(
    Connection* connection, bool assign)
        throw(StorageException)
            : IdStatement(connection, ToUndeliverableStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind((CONST text *)"UNDELIVERABLE", (sb4) 13*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_UNDELIVERABLE_STATE),
         (sb4) sizeof(SMSC_BYTE_UNDELIVERABLE_STATE));
}
void ToUndeliverableStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
    
    time_t cTime = time(NULL);
    convertDateToOCIDate(&(cTime), &currTime);
    bind(1, SQLT_ODT, (dvoid *) &(currTime), (sb4) sizeof(currTime));
}
void ToUndeliverableStatement::bindFailureCause(dvoid* cause, sb4 size)
    throw(StorageException)
{
    bind(2 , SQLT_UIN, cause, size);
}
void ToUndeliverableStatement::bindDestinationDescriptor(Descriptor& dst)
    throw(StorageException)
{
    indDstMsc = (!dst.mscLenght || !strlen(dst.msc)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;
    indDstImsi = (!dst.imsiLenght || !strlen(dst.imsi)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;

    bind(4 , SQLT_STR, (dvoid *) (dst.msc), 
         (sb4) sizeof(dst.msc), &indDstMsc);
    bind(5 , SQLT_STR, (dvoid *) (dst.imsi),
         (sb4) sizeof(dst.imsi), &indDstImsi);
    bind(6 , SQLT_UIN, (dvoid *)&(dst.sme),
         (sb4) sizeof(dst.sme));
}

const char* ToExpiredStatement::sql = (const char*)
"UPDATE SMS_MSG SET NEXT_TRY_TIME=NULL, ST=:EXPIRED\
 WHERE ID=:ID AND ST=:ENROUTE";
ToExpiredStatement::ToExpiredStatement(Connection* connection, bool assign)
        throw(StorageException)
            : IdStatement(connection, ToExpiredStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind((CONST text *)"EXPIRED", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_EXPIRED_STATE),
         (sb4) sizeof(SMSC_BYTE_EXPIRED_STATE));
}
void ToExpiredStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}

const char* ToDeletedStatement::sql = (const char*)
"UPDATE SMS_MSG SET NEXT_TRY_TIME=NULL, ST=:DELETED\
 WHERE ID=:ID AND ST=:ENROUTE";
ToDeletedStatement::ToDeletedStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ToDeletedStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    bind((CONST text *)"DELETED", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_DELETED_STATE),
         (sb4) sizeof(SMSC_BYTE_DELETED_STATE));
}
void ToDeletedStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_BIN, (dvoid *)&(smsId), sizeof(smsId));
}
    
/* --------------------- Sheduler's statements -------------------- */

const char* ReadyByNextTimeStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE ST=:ENROUTE\
 AND NEXT_TRY_TIME<=:RT ORDER BY NEXT_TRY_TIME ASC"; 
ReadyByNextTimeStatement::ReadyByNextTimeStatement(Connection* connection,
    bool assign) throw(StorageException)
        : IdStatement(connection, ReadyByNextTimeStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}
void ReadyByNextTimeStatement::bindRetryTime(time_t retryTime)
    throw(StorageException)
{
    convertDateToOCIDate(&(retryTime), &rTime);
    bind((CONST text *)"RT", (sb4) 2*sizeof(char), 
         SQLT_ODT, (dvoid *) &(rTime), (sb4) sizeof(rTime));
}

const char* MinNextTimeStatement::sql = (const char*)
"SELECT MIN(NEXT_TRY_TIME) FROM SMS_MSG WHERE ST=:ENROUTE"; 
MinNextTimeStatement::MinNextTimeStatement(Connection* connection,
    bool assign) throw(StorageException)
        : Statement(connection, MinNextTimeStatement::sql, assign)
{
    bind((CONST text *)"ENROUTE", (sb4) 7*sizeof(char),
         SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE),
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
    define(1, SQLT_ODT, (dvoid *) &(minNextTime),
           (sb4) sizeof(minNextTime));
}

time_t MinNextTimeStatement::getMinNextTime()
    throw(StorageException)
{
    time_t minTime;
    convertOCIDateToDate(&minNextTime, &(minTime));
    return minTime;
}

}}

