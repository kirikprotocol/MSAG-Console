#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DISABLE_TRACING
#define DISABLE_TRACING
#endif

#include "ConnectionManager.h"

namespace smsc { namespace store 
{

#ifndef SPARC
#define UINT64_SWAP_LE_BE_CONSTANT(val)   ((uint64_t) (\
     (((uint64_t) (val) << 32) | ((uint64_t) (val) >> 32))))
#else
#define UINT64_SWAP_LE_BE_CONSTANT(val)   ((uint64_t) (val))
#endif

/* ----------------------------- Statetment -------------------------- */

Statement::Statement(Connection* connection, const char* sql, bool assign) 
    throw(StorageException)
        : owner(connection), stmt(0L)
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
    if (stmt) {
        (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
    }
}

void Statement::bind(ub4 pos, ub2 type, 
                     dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    __trace2__("%p : BindByPos called, pos=%d", stmt, pos);

    OCIBind *bind = 0;
    check(OCIBindByPos(stmt, &bind, errhp, pos, 
                       placeholder, size, type, indp,
                       (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                       (ub4) OCI_DEFAULT));
}

void Statement::bind(CONST text* name, sb4 name_len, ub2 type,
                     dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    __trace2__("%p : BindByName called, name=%s", stmt, name);

    OCIBind *bind = 0;
    check(OCIBindByName(stmt, &bind, errhp, name, name_len,
                        placeholder, size, type, indp,
                        (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                        (ub4) OCI_DEFAULT));
}

void Statement::define(ub4 pos, ub2 type, 
                       dvoid* placeholder, sb4 size, dvoid* indp)
    throw(StorageException)
{
    __trace2__("%p : DefineByPos called, pos=%d", stmt, pos);

    OCIDefine*  define = 0;
    check(OCIDefineByPos(stmt, &define, errhp, pos, 
                         placeholder, size, type, indp,
                         (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT));
}

sword Statement::execute(ub4 mode, ub4 iters, ub4 rowoff)
{
    __trace2__("%p : Executing ...", stmt);

    sword result = OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                                  (CONST OCISnapshot *) NULL, 
                                  (OCISnapshot *) NULL, mode);
    if (result != OCI_SUCCESS && result != OCI_NO_DATA)
    {
        __trace2__("Error ocurred during executing statement %d", stmt);
    }

    return result;

    /*return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, 
                          (OCISnapshot *) NULL, mode);*/
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
    tm dt; gmtime_r(sms_date, &dt);

    OCIDateSetDate(oci_date, (sb2)(1900+dt.tm_year), 
                   (ub1)(1+dt.tm_mon), (ub1)(dt.tm_mday));
    OCIDateSetTime(oci_date, (ub1)(dt.tm_hour), 
                   (ub1)(dt.tm_min), (ub1)(dt.tm_sec));
}
void Statement::convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date)
{
    sb2 year;
    ub1 mon, mday, hour, min, sec;

    OCIDateGetTime(oci_date, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(oci_date, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    
    tm dt; dt.tm_isdst = 0;
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    *sms_date = mktime(&dt) - timezone;
}
void Statement::check(sword status) 
    throw(StorageException) 
{
    __require__(owner);
    owner->check(status);
}

/* ----------------------------- IdStatetment ------------------------- */  

void IdStatement::setSMSId(const SMSId _smsId)
    throw(StorageException)
{
    SMSId id = UINT64_SWAP_LE_BE_CONSTANT(_smsId);
    check(OCINumberFromInt(errhp, (CONST dvoid *)&id, 
                           sizeof(id), OCI_NUMBER_UNSIGNED, &smsId));
}
void IdStatement::getSMSId(SMSId &_smsId)
    throw(StorageException)
{
    SMSId id = 0;
    check(OCINumberToInt(errhp, (CONST OCINumber *)&smsId, 
                         sizeof(id), OCI_NUMBER_UNSIGNED, 
                         (dvoid *)&id));
    _smsId = UINT64_SWAP_LE_BE_CONSTANT(id);
}

/* --------------------------- GetIdStatement ----------------------- */
GetIdStatement::GetIdStatement(Connection* connection, const char* sql, 
                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    define(1, SQLT_VNU, (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* --------------------------- SetIdStatement ----------------------- */
SetIdStatement::SetIdStatement(Connection* connection, const char* sql,
                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    bind((CONST text*)"ID", (sb4) 2*sizeof(char), SQLT_VNU, 
         (dvoid *) &(smsId), (sb4) sizeof(smsId));
}

/* ----------------------- NeedRejectStatement ---------------------- */
const char* NeedRejectStatement::sql = (const char*)
"SELECT NVL(COUNT(*), 0) FROM SMS_MSG\
 WHERE MR=:MR AND OA=:OA AND DA=:DA";

NeedRejectStatement::NeedRejectStatement(Connection* connection, 
                                         bool assign)
    throw(StorageException)
        : Statement(connection, NeedRejectStatement::sql, assign)
{
    __trace2__("%p : NeedRejectStatement creating ...", stmt);

    define(1 , SQLT_UIN, (dvoid *) &(count), (sb4) sizeof(count));
}

void NeedRejectStatement::bindMr(dvoid* mr, sb4 size)
    throw(StorageException)
{
    bind(1 , SQLT_UIN, mr, size);
}
void NeedRejectStatement::bindOriginatingAddress(Address& address)
    throw(StorageException)
{
    convertAddressToString(address, oa);
    bind(2 , SQLT_STR, (dvoid *)(oa), (sb4) sizeof(oa));
}
void NeedRejectStatement::bindDestinationAddress(Address& address)
    throw(StorageException)
{
    convertAddressToString(address, da);
    bind(3 , SQLT_STR, (dvoid *)(da), (sb4) sizeof(da));
}

bool NeedRejectStatement::needReject()
{
    return ((count) ? true:false);
}

/* ----------------------- NeedOverwriteStatement ---------------------- */
const char* NeedOverwriteStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE\
 OA=:OA AND DA=:DA AND SVC_TYPE IS NULL";
NeedOverwriteStatement::NeedOverwriteStatement(Connection* connection, 
                                               bool assign)
    throw(StorageException)
        : IdStatement(connection, NeedOverwriteStatement::sql, assign)
{
    __trace2__("%p : NeedOverwriteStatement creating ...", stmt);

    define(1, SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
NeedOverwriteStatement::NeedOverwriteStatement(Connection* connection,
                                               const char* sql,
                                               bool assign)
    throw(StorageException)
        : IdStatement(connection, sql, assign)
{
    __trace2__("%p : NeedOverwriteStatement creating ...", stmt);

    define(1, SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}

void NeedOverwriteStatement::bindOriginatingAddress(Address& address)
    throw(StorageException)
{
    convertAddressToString(address, oa);
    bind(1 , SQLT_STR, (dvoid *)(oa), (sb4) sizeof(oa));
}
void NeedOverwriteStatement::bindDestinationAddress(Address& address)
    throw(StorageException)
{
    convertAddressToString(address, da);
    bind(2 , SQLT_STR, (dvoid *)(da), (sb4) sizeof(da));
}

void NeedOverwriteStatement::getId(SMSId& id)
    throw(StorageException)
{
    getSMSId(id);
}

const char* NeedOverwriteSvcStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE\
 OA=:OA AND DA=:DA AND SVC_TYPE=:SVC_TYPE";
NeedOverwriteSvcStatement::NeedOverwriteSvcStatement(Connection* connection, 
                                                     bool assign=true)
    throw(StorageException)
        : NeedOverwriteStatement(connection, 
                                 NeedOverwriteSvcStatement::sql, assign)
{
    __trace2__("%p : NeedOverwriteSvcStatement creating ...", stmt);
}
void NeedOverwriteSvcStatement::bindEServiceType(dvoid* type, sb4 size)
    throw(StorageException)
{
    bind(3 , SQLT_STR, type, size);
}

/* ------------------------- OverwriteStatement ---------------------- */
const char* OverwriteStatement::sql = (const char*)
"UPDATE SMS_MSG SET ID=:NEW_ID, MR=:MR,\
 SRC_MSC=:SRC_MSC, SRC_IMSI=:SRC_IMSI, SRC_SME_N=:SRC_SME_N,\
 DST_MSC=NULL, DST_IMSI=NULL, DST_SME_N=NULL, ATTEMPTS=0,\
 LAST_RESULT=0, LAST_TRY_TIME=NULL,\
 VALID_TIME=:VALID_TIME, SUBMIT_TIME=:SUBMIT_TIME,\
 NEXT_TRY_TIME=:NEXT_TRY_TIME, SVC_TYPE=:SVC,\
 DR=:DR, BR=:BR, ARC=:ARC, BODY=:BODY, BODY_LEN=:BODY_LEN,\
 ROUTE_ID=:ROUTE_ID, SVC_ID=:SVC_ID, PRTY=:PRTY,\
 SRC_SME_ID=:SRC_SME_ID, DST_SME_ID=:DST_SME_ID,\
 TXT_LENGTH=:TXT_LENGTH, MSG_REF=:MSG_REF, SEQ_NUM=:SEQ_NUM\
 WHERE ID=:OLD_ID";
OverwriteStatement::OverwriteStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, OverwriteStatement::sql, assign)
{
    __trace2__("%p : OverwriteStatement creating ...", stmt);
}
void OverwriteStatement::bindOldId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"OLD_ID", (sb4) 6*sizeof(char), 
         SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void OverwriteStatement::bindNewId(SMSId id)
    throw(StorageException)
{
    SMSId iid = UINT64_SWAP_LE_BE_CONSTANT(id);
    check(OCINumberFromInt(errhp, (CONST dvoid *)&iid, 
                           sizeof(iid), OCI_NUMBER_UNSIGNED, &newId));
    bind((CONST text *)"NEW_ID", (sb4) 6*sizeof(char), 
         SQLT_VNU, (dvoid *)&(newId), sizeof(newId));
}
void OverwriteStatement::bindSms(SMS& sms)
    throw(StorageException)
{
    ub4 i=2;

    bind(i++, SQLT_UIN, (dvoid *)&(sms.messageReference),
         (sb4) sizeof(sms.messageReference));
    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
         (sb4) sizeof(sms.originatingDescriptor.msc));
    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
         (sb4) sizeof(sms.originatingDescriptor.imsi));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.originatingDescriptor.sme),
         (sb4) sizeof(sms.originatingDescriptor.sme));
    
    convertDateToOCIDate(&(sms.validTime), &validTime);
    bind(i++, SQLT_ODT, (dvoid *) &(validTime), 
         (sb4) sizeof(validTime));
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    bind(i++, SQLT_ODT, (dvoid *) &(submitTime), 
         (sb4) sizeof(submitTime));
    
    if (sms.nextTime) 
    {
        convertDateToOCIDate(&(sms.nextTime), &nextTime);
        indNextTime = OCI_IND_NOTNULL;
    } 
    else indNextTime = OCI_IND_NULL;
    bind(i++, SQLT_ODT, (dvoid *) &(nextTime), 
         (sb4) sizeof(nextTime), &indNextTime);
    
    indSvcType = strlen(sms.eServiceType) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.eServiceType),
         (sb4) sizeof(sms.eServiceType), &indSvcType);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.deliveryReport),
         (sb4) sizeof(sms.deliveryReport));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.billingRecord),
         (sb4) sizeof(sms.billingRecord));
    bNeedArchivate = sms.needArchivate ? 'Y':'N';
    bind(i++, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
         (sb4) sizeof(bNeedArchivate));
    
    bodyBufferLen = sms.messageBody.getBufferLength();
    bodyBuffer = sms.messageBody.getBuffer();
    indBody = (bodyBuffer && bodyBufferLen <= MAX_BODY_LENGTH) 
                ? OCI_IND_NOTNULL : OCI_IND_NULL;
    bind(i++, SQLT_BIN, (dvoid *) bodyBuffer,
         (sb4) bodyBufferLen, &indBody);
    bind(i++, SQLT_UIN, (dvoid *)&(bodyBufferLen),
         (sb4) sizeof(bodyBufferLen));
    
    indRouteId = strlen(sms.routeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.routeId),
         (sb4) sizeof(sms.routeId), &indRouteId);
    bind(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
         (sb4) sizeof(sms.serviceId));
    bind(i++, SQLT_INT, (dvoid *)&(sms.priority),
         (sb4) sizeof(sms.priority));
    
    indSrcSmeId = strlen(sms.srcSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
         (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    indDstSmeId = strlen(sms.dstSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
         (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
    
    bodyTextLen = sms.messageBody.getShortMessageLength();
    bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen),
         (sb4) sizeof(bodyTextLen));
    
    try 
    {
        indMsgRef = (sms.hasBinProperty(Tag::SMSC_CONCATINFO)) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    } 
    catch (...) {
        throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
    }

    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatMsgRef),
         (sb4) sizeof(sms.concatMsgRef), &indMsgRef);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatSeqNum),
         (sb4) sizeof(sms.concatSeqNum));
}

/* --------------------------- StoreStatement ----------------------- */
const char* StoreStatement::sql = (const char*)
"INSERT INTO SMS_MSG (ID, ST, MR, OA, DA, DDA,\
 SRC_MSC, SRC_IMSI, SRC_SME_N,\
 VALID_TIME, SUBMIT_TIME, ATTEMPTS, LAST_RESULT,\
 LAST_TRY_TIME, NEXT_TRY_TIME, SVC_TYPE, DR, BR, ARC, BODY, BODY_LEN,\
 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, TXT_LENGTH, MSG_REF, SEQ_NUM)\
 VALUES (:ID, :ST, :MR, :OA, :DA, :DDA,\
 :SRC_MSC, :SRC_IMSI, :SRC_SME_N,\
 :VALID_TIME, :SUBMIT_TIME, 0, 0, NULL, :NEXT_TRY_TIME,\
 :SVC, :DR, :BR, :ARC, :BODY, :BODY_LEN,\
 :ROUTE_ID, :SVC_ID, :PRTY, :SRC_SME_ID, :DST_SME_ID,\
 :TXT_LENGTH, :MSG_REF, :SEQ_NUM)";
StoreStatement::StoreStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, StoreStatement::sql, assign)
{
    __trace2__("%p : StoreStatement creating ...", stmt);

    bind(2 , SQLT_UIN, (dvoid *) &(SMSC_BYTE_ENROUTE_STATE), 
         (sb4) sizeof(SMSC_BYTE_ENROUTE_STATE));
}
void StoreStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind(1 , SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void StoreStatement::bindSms(SMS& sms)
    throw(StorageException)
{
    ub4 i=3;

    bind(i++, SQLT_UIN, (dvoid *)&(sms.messageReference),
         (sb4) sizeof(sms.messageReference));
    
    convertAddressToString(sms.originatingAddress, oa);
    bind(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    convertAddressToString(sms.destinationAddress, da);
    bind(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    convertAddressToString(sms.dealiasedDestinationAddress, dda);
    bind(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));

    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
         (sb4) sizeof(sms.originatingDescriptor.msc));
    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
         (sb4) sizeof(sms.originatingDescriptor.imsi));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.originatingDescriptor.sme),
         (sb4) sizeof(sms.originatingDescriptor.sme));

    convertDateToOCIDate(&(sms.validTime), &validTime);
    bind(i++, SQLT_ODT, (dvoid *) &(validTime), 
         (sb4) sizeof(validTime));
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    bind(i++, SQLT_ODT, (dvoid *) &(submitTime), 
         (sb4) sizeof(submitTime));
    
    if (sms.nextTime) 
    {
        convertDateToOCIDate(&(sms.nextTime), &nextTime);
        indNextTime = OCI_IND_NOTNULL;
    } 
    else indNextTime = OCI_IND_NULL;
    bind(i++, SQLT_ODT, (dvoid *) &(nextTime), 
         (sb4) sizeof(nextTime), &indNextTime);
    
    indSvcType = strlen(sms.eServiceType) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.eServiceType),
         (sb4) sizeof(sms.eServiceType), &indSvcType);
    
    bind(i++, SQLT_UIN, (dvoid *)&(sms.deliveryReport),
         (sb4) sizeof(sms.deliveryReport));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.billingRecord),
         (sb4) sizeof(sms.billingRecord));
    bNeedArchivate = sms.needArchivate ? 'Y':'N';
    bind(i++, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
         (sb4) sizeof(bNeedArchivate));
    
    bodyBufferLen = sms.messageBody.getBufferLength();
    bodyBuffer = sms.messageBody.getBuffer();
    indBody = (bodyBuffer && bodyBufferLen <= MAX_BODY_LENGTH) 
                ? OCI_IND_NOTNULL : OCI_IND_NULL;
    bind(i++, SQLT_BIN, (dvoid *) bodyBuffer,
         (sb4) bodyBufferLen, &indBody);
    bind(i++, SQLT_UIN, (dvoid *)&(bodyBufferLen),
         (sb4) sizeof(bodyBufferLen));

    indRouteId = strlen(sms.routeId) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.routeId),
         (sb4) sizeof(sms.routeId), &indRouteId);
    bind(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
         (sb4) sizeof(sms.serviceId));
    bind(i++, SQLT_INT, (dvoid *)&(sms.priority),
         (sb4) sizeof(sms.priority));

    indSrcSmeId = strlen(sms.srcSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
         (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    indDstSmeId = strlen(sms.dstSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
         (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
    
    bodyTextLen = sms.messageBody.getShortMessageLength();
    bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen),
         (sb4) sizeof(bodyTextLen));

    try 
    {
        indMsgRef = (sms.hasBinProperty(Tag::SMSC_CONCATINFO)) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    } 
    catch (...) {
        throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
    }

    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatMsgRef),
         (sb4) sizeof(sms.concatMsgRef), &indMsgRef);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatSeqNum),
         (sb4) sizeof(sms.concatSeqNum));
}

/* --------------------------- RetrieveStatement ----------------------- */
const char* RetrieveStatement::sql = (const char*)
"SELECT ST, MR, OA, DA, DDA,\
 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N,\
 VALID_TIME, SUBMIT_TIME,\
 ATTEMPTS, LAST_RESULT, LAST_TRY_TIME, NEXT_TRY_TIME,\
 SVC_TYPE, DR, BR, ARC, BODY, BODY_LEN,\
 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, MSG_REF, SEQ_NUM\
 FROM SMS_MSG WHERE ID=:ID";
RetrieveStatement::RetrieveStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, RetrieveStatement::sql, assign)
{
    __trace2__("%p : RetrieveStatement creating ...", stmt);
}

void RetrieveStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind(1 , SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}

void RetrieveStatement::defineSms(SMS& sms)
    throw(StorageException)
{
    ub4 i=1;

    define(i++, SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    define(i++, SQLT_UIN, (dvoid *) &(sms.messageReference),
           (sb4) sizeof(sms.messageReference));
    
    define(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    define(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    define(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));

    define(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
           (sb4) sizeof(sms.originatingDescriptor.msc),
           (dvoid *)&(indSrcMsc));
    define(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
           (sb4) sizeof(sms.originatingDescriptor.imsi),
           (dvoid *)&(indSrcImsi));
    define(i++, SQLT_UIN, (dvoid *) &(sms.originatingDescriptor.sme),
           (sb4) sizeof(sms.originatingDescriptor.sme),
           (dvoid *)&(indSrcSme));
    define(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.msc),
           (sb4) sizeof(sms.destinationDescriptor.msc), 
           (dvoid *)&indDstMsc);
    define(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.imsi),
           (sb4) sizeof(sms.destinationDescriptor.imsi),
           (dvoid *)&(indDstImsi));
    define(i++, SQLT_UIN, (dvoid *) &(sms.destinationDescriptor.sme),
           (sb4) sizeof(sms.destinationDescriptor.sme),
           (dvoid *)&(indDstSme));
    define(i++, SQLT_ODT, (dvoid *) &(validTime),
           (sb4) sizeof(validTime));
    define(i++, SQLT_ODT, (dvoid *) &(submitTime),
           (sb4) sizeof(submitTime));
    define(i++, SQLT_UIN, (dvoid *) &(sms.attempts),
           (sb4) sizeof(sms.attempts));
    define(i++, SQLT_UIN, (dvoid *) &(sms.lastResult),
           (sb4) sizeof(sms.lastResult));
    define(i++, SQLT_ODT, (dvoid *) &(lastTime),
           (sb4) sizeof(lastTime),
           (dvoid *)&(indLastTime));
    define(i++, SQLT_ODT, (dvoid *) &(nextTime),
           (sb4) sizeof(nextTime),
           (dvoid *)&(indNextTime));
    define(i++, SQLT_STR, (dvoid *) &(sms.eServiceType),
           (sb4) sizeof(sms.eServiceType),
           (dvoid *) &(indSvc));
    define(i++, SQLT_UIN, (dvoid *) &(sms.deliveryReport),
           (sb4) sizeof(sms.deliveryReport));
    define(i++, SQLT_UIN, (dvoid *) &(sms.billingRecord),
           (sb4) sizeof(sms.billingRecord));
    define(i++, SQLT_AFC, (dvoid *) &(bNeedArchivate), 
           (sb4) sizeof(bNeedArchivate));
    define(i++, SQLT_BIN, (dvoid *) (bodyBuffer), 
           (sb4) sizeof(bodyBuffer), &indBody);
    define(i++, SQLT_UIN, (dvoid *) &(bodyBufferLen), 
           (sb4) sizeof(bodyBufferLen));
    
    define(i++, SQLT_STR, (dvoid *) (sms.routeId),
           (sb4) sizeof(sms.routeId), &indRouteId);
    define(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
           (sb4) sizeof(sms.serviceId));
    define(i++, SQLT_INT, (dvoid *)&(sms.priority),
           (sb4) sizeof(sms.priority));
    
    define(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
           (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    define(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
           (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
    
    define(i++, SQLT_UIN, (dvoid *) &(sms.concatMsgRef), 
           (sb4) sizeof(sms.concatMsgRef), &indMsgRef);
    define(i++, SQLT_UIN, (dvoid *) &(sms.concatSeqNum), 
           (sb4) sizeof(sms.concatSeqNum));
}

bool RetrieveStatement::getSms(SMS& sms)
{
    sms.state = (State) uState;
    sms.needArchivate = (bNeedArchivate == 'Y');
    
    convertStringToAddress(oa, sms.originatingAddress);
    convertStringToAddress(da, sms.destinationAddress);
    convertStringToAddress(dda, sms.dealiasedDestinationAddress);
    
    if (indSrcImsi == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.imsiLength 
            = strlen(sms.originatingDescriptor.imsi);
    } else {
        sms.originatingDescriptor.imsi[0] = '\0';
        sms.originatingDescriptor.imsiLength = 0;
    }
    if (indSrcMsc == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.mscLength 
            = strlen(sms.originatingDescriptor.msc);
    } else {
        sms.originatingDescriptor.msc[0] = '\0';
        sms.originatingDescriptor.mscLength = 0;
    }
    if (indDstImsi == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.imsiLength 
            = strlen(sms.destinationDescriptor.imsi);
    } else {
        sms.destinationDescriptor.imsi[0] = '\0';
        sms.destinationDescriptor.imsiLength = 0;
    }
    if (indDstMsc == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.mscLength 
            = strlen(sms.destinationDescriptor.msc);
    } else {
        sms.destinationDescriptor.msc[0] = '\0';
        sms.destinationDescriptor.mscLength = 0;
    }
    
    if (indSrcSme != OCI_IND_NOTNULL) 
        sms.originatingDescriptor.sme = 0;
    if (indDstSme != OCI_IND_NOTNULL) 
        sms.destinationDescriptor.sme = 0;
    if (indSvc != OCI_IND_NOTNULL) 
        sms.eServiceType[0] = '\0';
    if (indRouteId != OCI_IND_NOTNULL)
        sms.routeId[0] = '\0';
    if (indSrcSmeId != OCI_IND_NOTNULL)
        sms.srcSmeId[0] = '\0';
    if (indDstSmeId != OCI_IND_NOTNULL)
        sms.dstSmeId[0] = '\0';
    if (indMsgRef != OCI_IND_NOTNULL) {
        sms.concatMsgRef = 0;
        sms.concatSeqNum = 0;
    }
    
    if (indLastTime != OCI_IND_NOTNULL) sms.lastTime = 0;
    else convertOCIDateToDate(&lastTime, &(sms.lastTime));
    if (indNextTime != OCI_IND_NOTNULL) sms.nextTime = 0;
    else convertOCIDateToDate(&nextTime, &(sms.nextTime));
    
    convertOCIDateToDate(&submitTime, &(sms.submitTime));
    convertOCIDateToDate(&validTime, &(sms.validTime));

    bool result = (bodyBufferLen <= MAX_BODY_LENGTH);
    if (indBody != OCI_IND_NOTNULL || bodyBufferLen == 0)
    {
        sms.messageBody.setBuffer(0,0);
    }
    else 
    {
        uint8_t* setBuff = new uint8_t[bodyBufferLen];
        memcpy(setBuff, bodyBuffer, bodyBufferLen);
        sms.messageBody.setBuffer(setBuff, bodyBufferLen);
    }
    return result;
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
    bind(1 , SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}

/* ----------------------- RetrieveBodyStatement ---------------------- */
const char* RetrieveBodyStatement::sql = (const char*)
"SELECT BODY, BODY_LEN FROM SMS_MSG WHERE ID=:ID AND OA=:OA";
RetrieveBodyStatement::RetrieveBodyStatement(Connection* connection, bool assign) 
    throw(StorageException)
        : IdStatement(connection, RetrieveBodyStatement::sql, assign)
{
    __trace2__("%p : RetrieveBodyStatement creating ...", stmt);

    indBody = OCI_IND_NOTNULL;
    define(1 , SQLT_BIN, (dvoid *) (bodyBuffer), 
           (sb4) sizeof(bodyBuffer), &indBody);
    define(2 , SQLT_UIN, (dvoid *) &(bodyBufferLen), 
           (sb4) sizeof(bodyBufferLen));
}
          
void RetrieveBodyStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void RetrieveBodyStatement::bindOriginatingAddress(const Address& address)
    throw(StorageException)
{
    convertAddressToString(address, oa);
    bind((CONST text *)"OA", (sb4) 2*sizeof(char),
         SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
}
int RetrieveBodyStatement::getBodyLength(void)
    throw(StorageException)
{
    return bodyBufferLen;
}
bool RetrieveBodyStatement::getBody(Body& body)
    throw(StorageException)
{
    if (indBody != OCI_IND_NOTNULL) 
    { 
        body.setBuffer(0,0);
        return (bodyBufferLen == 0);
    }
    else 
    {
        uint8_t* setBuff = new uint8_t[bodyBufferLen];
        memcpy(setBuff, bodyBuffer, bodyBufferLen);
        body.setBuffer(setBuff, bodyBufferLen);
    }
    return true;
}

/* --------------------------- ReplaceStatement ----------------------- */        
const char* ReplaceStatement::sql = (const char*)
"UPDATE SMS_MSG SET\
 DR=:DR, BODY=:BODY, BODY_LEN=:BODY_LEN, TXT_LENGTH=:TXT_LENGTH\
 WHERE ID=:ID AND OA=:OA";
ReplaceStatement::ReplaceStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ReplaceStatement::sql, assign) 
{
    __trace2__("%p : ReplaceStatement creating ...", stmt);
}
ReplaceStatement::ReplaceStatement(Connection* connection, const char* sql,
                                   bool assign)
    throw(StorageException) 
        : IdStatement(connection, sql, assign) 
{
    __trace2__("%p : ReplaceStatement creating ...", stmt);
}

void ReplaceStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void ReplaceStatement::bindOriginatingAddress(const Address& address)
    throw(StorageException)
{
    convertAddressToString(address, oa);
    bind((CONST text *)"OA", (sb4) 2*sizeof(char),
         SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
}
void ReplaceStatement::bindBody(Body& body)
    throw(StorageException)
{
    bodyBufferLen = body.getBufferLength();
    bodyBuffer = body.getBuffer();
    indBody = (bodyBuffer && bodyBufferLen <= MAX_BODY_LENGTH) 
                ? OCI_IND_NOTNULL : OCI_IND_NULL;
    bodyTextLen = body.getShortMessageLength();

    bind((CONST text *)"BODY", (sb4) 4*sizeof(char),
         SQLT_BIN, (dvoid *) bodyBuffer,
         (sb4) bodyBufferLen, &indBody);
    bind((CONST text *)"BODY_LEN", (sb4) 8*sizeof(char),
         SQLT_UIN, (dvoid *)&(bodyBufferLen),
         (sb4) sizeof(bodyBufferLen));
    bind((CONST text *)"TXT_LENGTH", (sb4) 10*sizeof(char),
         SQLT_UIN, (dvoid *)&(bodyTextLen),
         (sb4) sizeof(bodyTextLen));
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
"UPDATE SMS_MSG SET\
 DR=:DR, BODY=:BODY, BODY_LEN=:BODY_LEN, TXT_LENGTH=:TXT_LENGTH,\
 VALID_TIME=:VT WHERE ID=:ID AND OA=:OA";
ReplaceVTStatement::ReplaceVTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVTStatement::sql, assign) {}

const char* ReplaceWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET\
 DR=:DR, BODY=:BODY, BODY_LEN=:BODY_LEN, TXT_LENGTH=:TXT_LENGTH,\
 NEXT_TRY_TIME=:WT WHERE ID=:ID AND OA=:OA";
ReplaceWTStatement::ReplaceWTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceWTStatement::sql, assign) {}

const char* ReplaceVWTStatement::sql = (const char*)
"UPDATE SMS_MSG SET\
 DR=:DR, BODY=:BODY, BODY_LEN=:BODY_LEN, TXT_LENGTH=:TXT_LENGTH,\
 VALID_TIME=:VT, NEXT_TRY_TIME=:WT WHERE ID=:ID AND OA=:OA";
ReplaceVWTStatement::ReplaceVWTStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceVWTStatement::sql, assign) {}

const char* ReplaceAllStatement::sql = (const char*)
"UPDATE SMS_MSG SET\
 DR=:DR, BODY=:BODY, BODY_LEN=:BODY_LEN, TXT_LENGTH=:TXT_LENGTH,\
 VALID_TIME=:VT, NEXT_TRY_TIME=:WT, MSG_REF=:MSG_REF, SEQ_NUM=:SEQ_NUM\
 WHERE ID=:ID AND OA=:OA";
ReplaceAllStatement::ReplaceAllStatement(Connection* connection, bool assign)
    throw(StorageException)
        : ReplaceStatement(connection, ReplaceAllStatement::sql, assign) {}

void ReplaceAllStatement::bindSms(SMS& sms)
    throw(StorageException)
{
    ub4 i=1;

    bind(i++, SQLT_UIN, (dvoid *)&(sms.deliveryReport), 
         (sb4) sizeof(sms.deliveryReport));
    
    bodyBufferLen = sms.messageBody.getBufferLength();
    bodyBuffer = sms.messageBody.getBuffer();
    indBody = (bodyBuffer && bodyBufferLen <= MAX_BODY_LENGTH) 
                ? OCI_IND_NOTNULL : OCI_IND_NULL;
    bodyTextLen = sms.messageBody.getShortMessageLength();
    bind(i++, SQLT_BIN, (dvoid *) bodyBuffer,
         (sb4) bodyBufferLen, &indBody);
    bind(i++, SQLT_UIN, (dvoid *)&(bodyBufferLen),
         (sb4) sizeof(bodyBufferLen));
    bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen),
         (sb4) sizeof(bodyTextLen));
    
    convertDateToOCIDate(&(sms.validTime), &vTime);
    bind(i++, SQLT_ODT, (dvoid *) &(vTime), (sb4) sizeof(vTime));
    convertDateToOCIDate(&(sms.nextTime), &wTime);
    bind(i++, SQLT_ODT, (dvoid *) &(wTime), (sb4) sizeof(wTime));
    
    try 
    {
        indMsgRef = (sms.hasBinProperty(Tag::SMSC_CONCATINFO)) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    } 
    catch (...) {
        throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
    }

    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatMsgRef),
         (sb4) sizeof(sms.concatMsgRef), &indMsgRef);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.concatSeqNum),
         (sb4) sizeof(sms.concatSeqNum));
    
    convertAddressToString(sms.originatingAddress, oa);
    bind((CONST text *)"OA", (sb4) 2*sizeof(char),
         SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
}

/* ------------------------ ChangeStateStatements ----------------------- */
const char* ToEnrouteStatement::sql = (const char*)
"UPDATE SMS_MSG SET ATTEMPTS=ATTEMPTS+:ATT, LAST_TRY_TIME=:CT,\
 NEXT_TRY_TIME=:NT, LAST_RESULT=:FCS,\
 DST_MSC=:MSC, DST_IMSI=:IMSI, DST_SME_N=:SME_N\
 WHERE ID=:ID";
ToEnrouteStatement::ToEnrouteStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ToEnrouteStatement::sql, assign)
{
    __trace2__("%p : ToEnrouteStatement creating ...", stmt);
}
void ToEnrouteStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void ToEnrouteStatement::bindAttemptsIncrement(uint8_t inc)
    throw(StorageException)
{
    attemptsIncrement = inc;
    bind(1 , SQLT_UIN, (dvoid *)&(attemptsIncrement),
         (sb4) sizeof(attemptsIncrement));
}
void ToEnrouteStatement::bindNextTime(time_t nextTryTime)
    throw(StorageException)
{
    time_t cTime = time(NULL);
    convertDateToOCIDate(&(cTime), &currTime);
    bind(2, SQLT_ODT, (dvoid *) &(currTime), (sb4) sizeof(currTime));
    convertDateToOCIDate(&(nextTryTime), &nextTime);
    bind(3, SQLT_ODT, (dvoid *) &(nextTime), (sb4) sizeof(nextTime));
}
void ToEnrouteStatement::bindFailureCause(dvoid* cause, sb4 size)
    throw(StorageException)
{
    bind(4, SQLT_UIN, cause, size);
}
void ToEnrouteStatement::bindDestinationDescriptor(Descriptor& dst)
    throw(StorageException)
{
    indDstMsc = (!dst.mscLength || !strlen(dst.msc)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;
    indDstImsi = (!dst.imsiLength || !strlen(dst.imsi)) ? 
                OCI_IND_NULL : OCI_IND_NOTNULL;

    bind(5 , SQLT_STR, (dvoid *) (dst.msc), 
         (sb4) sizeof(dst.msc), &indDstMsc);
    bind(6 , SQLT_STR, (dvoid *) (dst.imsi),
         (sb4) sizeof(dst.imsi), &indDstImsi);
    bind(7 , SQLT_UIN, (dvoid *)&(dst.sme),
         (sb4) sizeof(dst.sme));
}

/*
CREATE OR REPLACE PROCEDURE DO_FINALIZE_SMS                             
  (id IN NUMBER, st IN NUMBER, submitTime IN DATE, validTime IN DATE,   
   attempts IN NUMBER, lastResult IN NUMBER,                            
   lastTryTime IN DATE, nextTryTime IN DATE,                            
   oa IN VARCHAR2, oaVal IN VARCHAR2, oaTon IN NUMBER, oaNpi IN NUMBER, 
   da IN VARCHAR2, daVal IN VARCHAR2, daTon IN NUMBER, daNpi IN NUMBER, 
   dda IN VARCHAR2,                                                     
   mr IN NUMBER, svcType IN VARCHAR2, dr IN NUMBER, br IN NUMBER,       
   srcMsc IN VARCHAR2, srcImsi IN VARCHAR2, srcSmeN IN NUMBER,          
   dstMsc IN VARCHAR2, dstImsi IN VARCHAR2, dstSmeN IN NUMBER,          
   routeId IN VARCHAR2, svcId IN NUMBER, prty IN NUMBER,                
   srcSmeId IN VARCHAR2, dstSmeId VARCHAR2, txtLength IN NUMBER,        
   bodyLen IN NUMBER, body IN RAW, arc IN NUMBER, bill IN NUMBER)       
*/
const char* ToFinalStatement::sql = (const char*)
"BEGIN\
    DO_FINALIZE_SMS\
    (:id, :st, :submitTime, :validTime, :attempts,\
     :lastResult, :lastTryTime, :nextTryTime,\
     :oa, :oaVal, :oaTon, :oaNpi, :da, :daVal, :daTon, :daNpi, :dda,\
     :mr, :svcType, :dr, :br,\
     :srcMsc, :srcImsi, :srcSmeN, :dstMsc, :dstImsi, :dstSmeN,\
     :routeId, :svcId, :prty, :srcSmeId, :dstSmeId,\
     :txtLength, :bodyLen, :body, :arc, :bill);\
 END;";
ToFinalStatement::ToFinalStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, ToFinalStatement::sql, assign)
{
    __trace2__("%p : FinalizeStatement creating ...", stmt);
}
void ToFinalStatement::bindSms(SMSId id, SMS& sms)
    throw(StorageException)
{
    setSMSId(id);

    ub4 i=1;
    bind(i++, SQLT_VNU, (dvoid *)&(smsId), (sb4)sizeof(smsId));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.state), (sb4)sizeof(sms.state));
    convertDateToOCIDate(&(sms.submitTime), &submitTime);
    bind(i++, SQLT_ODT, (dvoid *) &(submitTime), (sb4)sizeof(submitTime));
    convertDateToOCIDate(&(sms.validTime), &validTime);
    bind(i++, SQLT_ODT, (dvoid *) &(validTime), (sb4)sizeof(validTime));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.attempts), (sb4)sizeof(sms.attempts));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.lastResult), (sb4)sizeof(sms.lastResult));
    
    time_t cTime = time(NULL);
    if (sms.lastTime) {
        convertDateToOCIDate(&(sms.lastTime), &lastTime);
        indLastTime = OCI_IND_NOTNULL;
    } 
    else indLastTime = OCI_IND_NULL;
    bind(i++, SQLT_ODT, (dvoid *) &(lastTime), 
         (sb4)sizeof(lastTime), &indLastTime);
    if (sms.nextTime) {
        convertDateToOCIDate(&(sms.nextTime), &nextTime);
        indNextTime = OCI_IND_NOTNULL;
    } 
    else indNextTime = OCI_IND_NULL;
    bind(i++, SQLT_ODT, (dvoid *) &(nextTime), 
         (sb4)sizeof(nextTime), &indNextTime);
    
    // OA & OA VAL, TON, NPI
    convertAddressToString(sms.originatingAddress, oa);
    bind(i++, SQLT_STR, (dvoid *) (oa), (sb4)sizeof(oa));
    bind(i++, SQLT_STR, (dvoid *) sms.originatingAddress.value, 
         (sb4)sizeof(AddressValue));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.originatingAddress.type), 
         (sb4)sizeof(sms.originatingAddress.type));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.originatingAddress.plan), 
         (sb4)sizeof(sms.originatingAddress.plan));
    // DA & DDA VAL, TON, NPI
    convertAddressToString(sms.destinationAddress, da);
    bind(i++, SQLT_STR, (dvoid *) (da), (sb4)sizeof(da));
    bind(i++, SQLT_STR, (dvoid *) (sms.dealiasedDestinationAddress.value), 
         (sb4)sizeof(sms.dealiasedDestinationAddress.value));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.dealiasedDestinationAddress.type), 
         (sb4)sizeof(sms.dealiasedDestinationAddress.type));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.dealiasedDestinationAddress.plan), 
         (sb4)sizeof(sms.dealiasedDestinationAddress.plan));
    // DDA
    convertAddressToString(sms.dealiasedDestinationAddress, dda);
    bind(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));
    
    bind(i++, SQLT_UIN, (dvoid *)&(sms.messageReference),
         (sb4)sizeof(sms.messageReference));
    
    indSvcType = strlen(sms.eServiceType) ? 
                    OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.eServiceType),
         (sb4)sizeof(sms.eServiceType), &indSvcType);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.deliveryReport),
         (sb4)sizeof(sms.deliveryReport));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.billingRecord),
         (sb4)sizeof(sms.billingRecord));

    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
         (sb4) sizeof(sms.originatingDescriptor.msc));
    bind(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
         (sb4) sizeof(sms.originatingDescriptor.imsi));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.originatingDescriptor.sme),
         (sb4) sizeof(sms.originatingDescriptor.sme));

    indDstMsc = (!sms.destinationDescriptor.mscLength || 
                 !strlen(sms.destinationDescriptor.msc)) ? 
                    OCI_IND_NULL : OCI_IND_NOTNULL;
    indDstImsi = (!sms.destinationDescriptor.imsiLength || 
                  !strlen(sms.destinationDescriptor.imsi)) ? 
                    OCI_IND_NULL : OCI_IND_NOTNULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.msc),
         (sb4) sizeof(sms.destinationDescriptor.msc), &indDstMsc);
    bind(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.imsi),
         (sb4) sizeof(sms.destinationDescriptor.imsi), &indDstImsi);
    bind(i++, SQLT_UIN, (dvoid *)&(sms.destinationDescriptor.sme),
         (sb4) sizeof(sms.destinationDescriptor.sme));

    indRouteId = strlen(sms.routeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.routeId),
         (sb4)sizeof(sms.routeId), &indRouteId);
    bind(i++, SQLT_INT, (dvoid *)&(sms.serviceId),
         (sb4)sizeof(sms.serviceId));
    bind(i++, SQLT_INT, (dvoid *)&(sms.priority),
         (sb4)sizeof(sms.priority));
    
    indSrcSmeId = strlen(sms.srcSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),
         (sb4)sizeof(sms.srcSmeId), &indSrcSmeId);
    indDstSmeId = strlen(sms.dstSmeId) ? OCI_IND_NOTNULL:OCI_IND_NULL;
    bind(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),
         (sb4)sizeof(sms.dstSmeId), &indDstSmeId);

    bodyTextLen = sms.messageBody.getShortMessageLength();
    bind(i++, SQLT_UIN, (dvoid *)&(bodyTextLen), (sb4)sizeof(bodyTextLen));

    bodyBufferLen = sms.messageBody.getBufferLength();
    bodyBuffer = sms.messageBody.getBuffer();
    indBody = (bodyBuffer && bodyBufferLen <= MAX_BODY_LENGTH) 
                ? OCI_IND_NOTNULL : OCI_IND_NULL;
    bind(i++, SQLT_UIN, (dvoid *)&(bodyBufferLen),
         (sb4) sizeof(bodyBufferLen));
    bind(i++, SQLT_BIN, (dvoid *) bodyBuffer,
         (sb4) bodyBufferLen, &indBody);
    
    bind(i++, SQLT_UIN, (dvoid *)&(sms.needArchivate), // arc
         (sb4) sizeof(sms.needArchivate));
    bind(i++, SQLT_UIN, (dvoid *)&(sms.billingRecord), // bill
         (sb4)sizeof(sms.billingRecord));
}


const char* UpdateSeqNumStatement::sql = (const char*)
"UPDATE SMS_MSG SET SEQ_NUM=SEQ_NUM+:INC\
 WHERE ID=:ID AND MSG_REF IS NOT NULL";
UpdateSeqNumStatement::UpdateSeqNumStatement(Connection* connection, bool assign)
    throw(StorageException)
        : IdStatement(connection, UpdateSeqNumStatement::sql, assign)
{
    __trace2__("%p : UpdateSeqNumStatement creating ...", stmt);
}
void UpdateSeqNumStatement::bindId(SMSId id)
    throw(StorageException)
{
    setSMSId(id);
    bind((CONST text *)"ID", (sb4) 2*sizeof(char), 
         SQLT_VNU, (dvoid *)&(smsId), sizeof(smsId));
}
void UpdateSeqNumStatement::bindInc(int8_t inc)
    throw(StorageException)
{
    incVal = inc;
    bind((CONST text *)"INC", (sb4) 3*sizeof(char), 
         SQLT_UIN, (dvoid *)&(incVal), sizeof(incVal));
}

const char* ConcatDataStatement::sql = (const char*)
"SELECT MSG_REF FROM SMS_MSG WHERE ID =\
(SELECT MAX(T.ID) FROM SMS_MSG T WHERE T.MSG_REF IS NOT NULL AND T.DDA=:DDA)";
ConcatDataStatement::ConcatDataStatement(Connection* connection, bool assign)
    throw(StorageException)
        : Statement(connection, ConcatDataStatement::sql, assign)
{
    __trace2__("%p : ConcatDataStatement creating ...", stmt);
    
    dstAddr[0] = '\0'; msgRef = 0;
    define(1, SQLT_UIN, (dvoid *) &(msgRef), (sb4) sizeof(msgRef));
}
void ConcatDataStatement::setDestination(const Address& dda)
{
    convertAddressToString(dda, dstAddr);   
    bind(1, SQLT_STR, (dvoid *) (dstAddr), (sb4) sizeof(dstAddr));
}


/* --------------------- Sheduler's statements -------------------- */

const char* ReadyByNextTimeStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE\
 NEXT_TRY_TIME<=:RT ORDER BY NEXT_TRY_TIME ASC"; 
ReadyByNextTimeStatement::ReadyByNextTimeStatement(Connection* connection,
    bool assign) throw(StorageException)
        : IdStatement(connection, ReadyByNextTimeStatement::sql, assign)
{
    __trace2__("%p : ReadyByNextTimeStatement creating ...", stmt);
    
    define(1, SQLT_VNU, (dvoid *) &(smsId), (sb4) sizeof(smsId)); 
}
void ReadyByNextTimeStatement::bindRetryTime(time_t retryTime)
    throw(StorageException)
{
    convertDateToOCIDate(&(retryTime), &rTime);
    bind((CONST text *)"RT", (sb4) 2*sizeof(char), 
         SQLT_ODT, (dvoid *) &(rTime), (sb4) sizeof(rTime));
}

const char* MinNextTimeStatement::sql = (const char*)
"SELECT MIN(NEXT_TRY_TIME) FROM SMS_MSG"; 
MinNextTimeStatement::MinNextTimeStatement(Connection* connection,
    bool assign) throw(StorageException)
        : Statement(connection, MinNextTimeStatement::sql, assign)
{
    __trace2__("%p : MinNextTimeStatement creating ...", stmt);

    indNextTime = OCI_IND_NOTNULL;
    define(1 , SQLT_ODT, (dvoid *) &(minNextTime),
           (sb4) sizeof(minNextTime), (dvoid *)&indNextTime);
}

time_t MinNextTimeStatement::getMinNextTime()
    throw(StorageException)
{
    time_t minTime = 0L;
    if (indNextTime == OCI_IND_NOTNULL)
    {
        convertOCIDateToDate(&minNextTime, &minTime);
    }
    return minTime;
}

const char* CancelIdsStatement::sql1 = (const char*)
"SELECT ID FROM SMS_MSG WHERE\
 OA=:OA AND DA=:DA AND SVC_TYPE=:SVC";
const char* CancelIdsStatement::sql2 = (const char*)
"SELECT ID FROM SMS_MSG WHERE\
 OA=:OA AND DA=:DA"; //  AND SVC_TYPE IS NULL ???
CancelIdsStatement::CancelIdsStatement(Connection* connection,
    const Address& _oa, const Address& _da, const char* svc, bool assign)
        throw(StorageException) 
        : IdStatement(connection, ((svc) ? CancelIdsStatement::sql1: 
                                           CancelIdsStatement::sql2), assign)
{
    ub4 i=1; 
    
    convertAddressToString(_oa, oa);
    bind(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    convertAddressToString(_da, da);
    bind(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    
    if (svc)
    {
        bind(i++, SQLT_STR, (dvoid *)svc, (sb4) strlen(svc)+1);
    }
    
    define(1, SQLT_VNU, (dvoid *) &(smsId), (sb4) sizeof(smsId)); 
}

const char* DeliveryIdsStatement::sql = (const char*)
"SELECT ID FROM SMS_MSG WHERE ATTEMPTS>0 AND DDA=:DA"; 
DeliveryIdsStatement::DeliveryIdsStatement(Connection* connection, 
                                           const Address& _da, bool assign)
    throw(StorageException)
    : IdStatement(connection, DeliveryIdsStatement::sql, assign)
{
    convertAddressToString(_da, da);
    bind(1, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    define(1, SQLT_VNU, (dvoid *) &(smsId), (sb4) sizeof(smsId)); 
}

/* --------------------- Body (BLOB) statements -------------------- */

BodyStatement::BodyStatement(Connection* connection, const char* sql, 
                             bool assign=false) 
    throw(StorageException)
        : SetIdStatement(connection, sql, assign), 
            indBody(OCI_IND_NOTNULL)
{
    // Allocate locator descriptor
    check(OCIDescriptorAlloc((dvoid *)envhp, (dvoid **)&locator,
                             (ub4)OCI_DTYPE_LOB, (size_t)0, (dvoid **)0));
}
BodyStatement::~BodyStatement()

{
    // Free locator descriptor
    (void) OCIDescriptorFree((dvoid *) locator, (ub4) OCI_DTYPE_LOB);
}

const char* SetBodyStatement::sql = (const char*)
"INSERT INTO SMS_ATCH (ID, BODY) VALUES (:ID, EMPTY_BLOB()) "
"RETURNING BODY INTO :BODY";
SetBodyStatement::SetBodyStatement(Connection* connection, bool assign)
    throw(StorageException) 
        : BodyStatement(connection, SetBodyStatement::sql, assign)
{
    __trace2__("%p : SetBodyStatement creating ...", stmt);

    bind((CONST text *)"BODY", (sb4) 4*sizeof(char),
         SQLT_BLOB, (dvoid *) &locator, (sb4) 0, &indBody);
}

void SetBodyStatement::setBody(Body& body)
    throw(StorageException)
{
    __trace2__("%p : Writing to BLOB ...", stmt);

    check(execute(OCI_DEFAULT, 1, 0));

    int isOpen = false;
    ub4 amount = body.getBufferLength();
    ub4 offset = 1;

    check(OCILobOpen(svchp, errhp, locator, OCI_LOB_READWRITE));
    check(OCILobIsOpen(svchp, errhp, locator, &isOpen));
    
    if (!isOpen) throw StorageException("Can't open SMS_ATCH::LOB for writing.");
    
    uint8_t* buff = body.getBuffer();

    if (amount && buff)
    {
        ub4 size = amount;

        check(OCILobWrite(svchp, errhp, locator, &amount, offset, 
                          (dvoid *)buff, amount, OCI_ONE_PIECE, (dvoid *)0, 
                          (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
                          (ub2) 0, (ub1) 0));

        if (size != amount)
            throw StorageException("Can't write %d bytes to SMS_ATCH::LOB. "
                                   "Only %d bytes written.", size, amount);
    }
    check(OCILobClose(svchp, errhp, locator));
}

const char* GetBodyStatement::sql = (const char*)
"SELECT BODY FROM SMS_ATCH WHERE ID=:ID FOR UPDATE";
GetBodyStatement::GetBodyStatement(Connection* connection, bool assign)
    throw(StorageException) 
        : BodyStatement(connection, GetBodyStatement::sql, assign)
{
    __trace2__("%p : GetBodyStatement creating ...", stmt);

    define(1, SQLT_BLOB, (dvoid *) &locator, (sb4) 0, &indBody);
}

bool GetBodyStatement::getBody(Body& body)
    throw(StorageException)
{
    __trace2__("%p : Reading from BLOB ...", stmt);

    check(execute(OCI_DEFAULT, 1, 0));

    bool ret = false;
    if (indBody != OCI_IND_NOTNULL)
    {
        body.setBuffer(0, 0);
        return false;
    }

    int isOpen = false;
    ub4 amount = 0;
    ub4 offset = 1;

    check(OCILobOpen(svchp, errhp, locator, OCI_LOB_READONLY));
    check(OCILobIsOpen(svchp, errhp, locator, &isOpen));
    
    if (!isOpen) throw StorageException("Can't open SMS_ATCH::LOB for reading.");

    check(OCILobGetLength (svchp, errhp, locator, &amount));
    if (amount)
    {
        ub4 size = amount;
        uint8_t* buff = new uint8_t[amount];

        check(OCILobRead(svchp, errhp, locator, &amount, offset,
                         (dvoid *)buff, size, (dvoid *)0, 0,
                         (ub2) 0, (ub1) 0));

        if (size != amount)
            throw StorageException("Can't read %d bytes from SMS_ATCH::LOB. "
                                   "Only %d bytes read.", size, amount);

        body.setBuffer(buff, size);
        ret = true;
    }
    else 
    {
        body.setBuffer(0, 0);
        ret = false;
    }
    
    check(OCILobClose(svchp, errhp, locator));
    return ret;
}

const char* DestroyBodyStatement::sql = (const char*)
"DELETE FROM SMS_ATCH WHERE ID=:ID";
DestroyBodyStatement::DestroyBodyStatement(Connection* connection, 
                                           bool assign=true)
    throw(StorageException)
        : BodyStatement(connection, DestroyBodyStatement::sql, assign)
{
    __trace2__("%p : DestroyBodyStatement creating ...", stmt);
}
bool DestroyBodyStatement::destroyBody()
    throw(StorageException)
{
    check(execute(OCI_DEFAULT, 1, 0));
    return (getRowsAffectedCount() ? true:false);
}

}}

