#include <stdio.h>
#include <stdlib.h>

#include <orl.h>
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

void checkerror(OCIError *errhp, sword status)
{
    text errbuf[512];
    ub4 buflen;
    ub4 errcode;
    
    switch (status)
    {
    case OCI_SUCCESS:
	break;
    case OCI_SUCCESS_WITH_INFO:
	(void) printf("Error - OCI_SUCCESS_WITH_INFO\n");
	break;
    case OCI_NEED_DATA:
	(void) printf("Error - OCI_NEED_DATA\n");
	break;
    case OCI_NO_DATA:
	(void) printf("Error - OCI_NODATA\n");
	break;
    case OCI_ERROR:
	(void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL, (sb4 *)&errcode,
	errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
	(void) printf("Error - %s\n", errbuf);
	break;
    case OCI_INVALID_HANDLE:
	(void) printf("Error - OCI_INVALID_HANDLE\n");
	break;
    case OCI_STILL_EXECUTING:
	(void) printf("Error - OCI_STILL_EXECUTE\n");
	break;
    default:
	break;
    }	
}

SMSId StoreManager::store(SMS* sms)
    throw(ResourceAllocationException)
{
    Connection* conn;
    if (sms && pool && (conn = pool->getConnection())) 
    {
        SMSId   id;
        sword   status;

        // lock table
        status = OCIStmtExecute(conn->svchp, conn->storeStmthps[0], conn->errhp,
                                (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) 
        {
            checkerror(conn->errhp, status);
            throw ResourceAllocationException(); // ??? what exception should i throw here ??? 
        }
        
        // define placeholder for max(id)
        OCIDefine*  defhp;
        (void) OCIDefineByPos(conn->storeStmthps[1], &defhp, conn->errhp,
                              1, (dvoid *) &id, (sword) sizeof(id), SQLT_INT,
                              (dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
        // "select max(id) from sms_msg"
        status = OCIStmtExecute(conn->svchp, conn->storeStmthps[1], conn->errhp,
                                (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT);
        if (status != OCI_SUCCESS) 
        {
            /*if (status == OCI_NO_DATA) {
                id = 0;
            } else {
                checkerror(conn->errhp, status);
                throw ResourceAllocationException(); // ??? what exception should i throw here ??? 
            }*/
            id = 0;
        }
        id++;
        printf("SMS id: %d\n", id);
        
        // bind sms fields to statement
        OCIBind*    bndId;
        status = OCIBindByPos(conn->storeStmthps[2], &bndId, conn->errhp,(ub4) 1,
                     (dvoid *) &id,(sb4) sizeof(id), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndState;
        uint8_t st = (uint8_t)sms->getState(); 
        status = OCIBindByPos(conn->storeStmthps[2], &bndState, conn->errhp,(ub4) 2,
                     (dvoid *) &(st),(sb4) sizeof(st), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndOALen;
        Address oa = sms->getOriginatingAddress(); 
        uint8_t oaLen = oa.getLenght();
        status = OCIBindByPos(conn->storeStmthps[2], &bndOALen, conn->errhp,(ub4) 3,
                     (dvoid *) &(oaLen),(sb4) sizeof(oaLen), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndOATon;
        uint8_t oaTon = oa.getTypeOfNumber();
        status = OCIBindByPos(conn->storeStmthps[2], &bndOATon, conn->errhp,(ub4) 4,
                     (dvoid *) &(oaTon),(sb4) sizeof(oaTon), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndOANpi;
        uint8_t oaNpi = oa.getNumberingPlan();
        status = OCIBindByPos(conn->storeStmthps[2], &bndOANpi, conn->errhp,(ub4) 5,
             (dvoid *) &(oaNpi),(sb4) sizeof(oaNpi), SQLT_INT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndOAVal;
        status = OCIBindByPos(conn->storeStmthps[2], &bndOAVal, conn->errhp,(ub4) 6,
             (dvoid *)(oa.value),(sb4) sizeof(oa.value), SQLT_CHR, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndDALen;
        Address da = sms->getDestinationAddress(); 
        uint8_t daLen = da.getLenght();
        status = OCIBindByPos(conn->storeStmthps[2], &bndDALen, conn->errhp,(ub4) 7,
                     (dvoid *) &(daLen),(sb4) sizeof(daLen), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndDATon;
        uint8_t daTon = da.getTypeOfNumber();
        status = OCIBindByPos(conn->storeStmthps[2], &bndDATon, conn->errhp,(ub4) 8,
                     (dvoid *) &(daTon),(sb4) sizeof(daTon), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndDANpi;
        uint8_t daNpi = da.getNumberingPlan();
        status = OCIBindByPos(conn->storeStmthps[2], &bndDANpi, conn->errhp,(ub4) 9,
             (dvoid *) &(daNpi),(sb4) sizeof(daNpi), SQLT_INT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndDAVal;
        status = OCIBindByPos(conn->storeStmthps[2], &bndDAVal, conn->errhp,(ub4) 10,
             (dvoid *)(da.value),(sb4) sizeof(da.value), SQLT_CHR, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        tm*         dt;
        
        OCIDate     validTime;
        dt = localtime(&(sms->validTime));
        OCIDateSetDate(&validTime, (sb2)(1900+dt->tm_year), 
                       (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
        OCIDateSetTime(&validTime, (ub1)(dt->tm_hour), 
                       (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
        OCIBind*    bndVTime;
        status = OCIBindByPos(conn->storeStmthps[2], &bndVTime, conn->errhp,(ub4) 11,
             (dvoid *)(&validTime),(sb4) sizeof(validTime), SQLT_ODT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIDate     waitTime;
        dt = localtime(&(sms->waitTime));
        OCIDateSetDate(&waitTime, (sb2)(1900+dt->tm_year), 
                       (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
        OCIDateSetTime(&waitTime, (ub1)(dt->tm_hour), 
                       (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
        OCIBind*    bndWTime;
        status = OCIBindByPos(conn->storeStmthps[2], &bndWTime, conn->errhp,(ub4) 12,
             (dvoid *)(&waitTime),(sb4) sizeof(waitTime), SQLT_ODT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIDate     submitTime;
        dt = localtime(&(sms->submitTime));
        OCIDateSetDate(&submitTime, (sb2)(1900+dt->tm_year), 
                       (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
        OCIDateSetTime(&submitTime, (ub1)(dt->tm_hour), 
                       (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
        OCIBind*    bndSTime;
        status = OCIBindByPos(conn->storeStmthps[2], &bndSTime, conn->errhp,(ub4) 13,
             (dvoid *)(&submitTime),(sb4) sizeof(submitTime), SQLT_ODT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIDate     deliveryTime;
        dt = localtime(&(sms->deliveryTime));
        OCIDateSetDate(&deliveryTime, (sb2)(1900+dt->tm_year), 
                       (ub1)(1+dt->tm_mon), (ub1)(dt->tm_mday));
        OCIDateSetTime(&deliveryTime, (ub1)(dt->tm_hour), 
                       (ub1)(dt->tm_min), (ub1)(dt->tm_sec));
        OCIBind*    bndDTime;
        status = OCIBindByPos(conn->storeStmthps[2], &bndDTime, conn->errhp,(ub4) 14,
             (dvoid *)(&deliveryTime),(sb4) sizeof(deliveryTime), SQLT_ODT, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndMsgRef;
        uint8_t msgRef = sms->getMessageReference();
        status = OCIBindByPos(conn->storeStmthps[2], &bndMsgRef, conn->errhp,(ub4) 15,
                     (dvoid *) &(msgRef),(sb4) sizeof(msgRef), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndMsgIdent;
        uint8_t msgIdent = sms->getMessageIdentifier();
        status = OCIBindByPos(conn->storeStmthps[2], &bndMsgIdent, conn->errhp,(ub4) 16,
                     (dvoid *) &(msgIdent),(sb4) sizeof(msgIdent), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndSrr;
        char bSrr = (sms->isStatusReportRequested() ? 'Y':'N');
        status = OCIBindByPos(conn->storeStmthps[2], &bndSrr, conn->errhp,(ub4) 17,
                     (dvoid *) &(bSrr),(sb4) sizeof(bSrr), SQLT_AFC, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        
        OCIBind*    bndRd;
        char bRd = (sms->isRejectDuplicates() ? 'Y':'N');
        status = OCIBindByPos(conn->storeStmthps[2], &bndRd, conn->errhp,(ub4) 18,
                     (dvoid *) &(bRd),(sb4) sizeof(bRd), SQLT_AFC, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        checkerror(conn->errhp, status);
        OCIBind*    bndMsgPri;
        uint8_t msgPri = sms->getPriority();
        status = OCIBindByPos(conn->storeStmthps[2], &bndMsgPri, conn->errhp,(ub4) 19,
                     (dvoid *) &(msgPri),(sb4) sizeof(msgPri), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        OCIBind*    bndMsgPid;
        uint8_t msgPid = sms->getProtocolIdentifier();
        status = OCIBindByPos(conn->storeStmthps[2], &bndMsgPid, conn->errhp,(ub4) 20,
                     (dvoid *) &(msgPid),(sb4) sizeof(msgPid), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndFcs;
        uint8_t fcs = sms->getFailureCause();
        status = OCIBindByPos(conn->storeStmthps[2], &bndFcs, conn->errhp,(ub4) 21,
                     (dvoid *) &(fcs),(sb4) sizeof(fcs), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndDcs;
        Body body = sms->getMessageBody();
        uint8_t dcs = body.getCodingScheme();
        status = OCIBindByPos(conn->storeStmthps[2], &bndDcs, conn->errhp,(ub4) 22,
                     (dvoid *) &(dcs),(sb4) sizeof(dcs), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndUdhi;
        char udhi = ((body.isHeaderIndicator()) ? 'Y':'N');
        status = OCIBindByPos(conn->storeStmthps[2], &bndUdhi, conn->errhp,(ub4) 23,
                     (dvoid *) &(udhi),(sb4) sizeof(udhi), SQLT_AFC, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndUdl;
        uint8_t udl = body.lenght;
        status = OCIBindByPos(conn->storeStmthps[2], &bndUdl, conn->errhp,(ub4) 24,
                     (dvoid *) &(udl),(sb4) sizeof(udl), SQLT_INT, (dvoid *) 0,
                     (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        
        OCIBind*    bndUd;
        status = OCIBindByPos(conn->storeStmthps[2], &bndUd, conn->errhp,(ub4) 25,
             (dvoid *)(body.data),(sb4) sizeof(body.data), SQLT_CHR, (dvoid *) 0,
             (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
        checkerror(conn->errhp, status);

        // insert new sms row into table
        status = OCIStmtExecute(conn->svchp, conn->storeStmthps[2], conn->errhp,
                                (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT);
        printf("Execute: %d\n", status);
        checkerror(conn->errhp, status);

        status = OCITransCommit (conn->svchp, conn->errhp, OCI_DEFAULT);
        checkerror(conn->errhp, status);
        printf("Commit: %d\n", status);

        pool->freeConnection(conn);
        return id;
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

