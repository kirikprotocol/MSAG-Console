#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>
#include "ConnectionManager.h"

namespace smsc { namespace store 
{
using namespace smsc::sms;

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
ConnectionPool::ConnectionPool(StoreConfig* _config) throw(StoreException)
	: config(_config)/*, connectionsCount(10)*/
{
    if (!config) {
		throw AuthenticationException();
    }
    
	/*idle.SetSize(connectionsCount);
	busy.SetSize(connectionsCount);
	dead.SetSize(connectionsCount);*/
	
	connectionsCount = 10;
	
	for (int i=0; i<connectionsCount; i++)
	{
		Connection*	connection = new Connection(this, i);
		(void) idle.Push(connection);
	}
}

ConnectionPool::~ConnectionPool()
{
	MutexGuard	guard(connectionsLock);

	for (int i=0; i<idle.Count(); i++)
	{
		Connection*	connection=0L;
		(void) idle.Pop(connection);
		if (connection) delete connection;
	}
	for (int i=0; i<busy.Count(); i++)
	{
		Connection*	connection=0L;
		(void) busy.Pop(connection);
		if (connection) delete connection;
	}
	for (int i=0; i<dead.Count(); i++) 
	{
		Connection*	connection=0L;
		(void) dead.Pop(connection);
		if (connection) delete connection;
	}
    if (config) delete config; 
}

void ConnectionPool::checkErr(sword status, Connection* conn) 
	throw(StoreException)
{
    OCIError*	errhp;
	text		errbuf[512];
    ub4			buflen, errcode;
    
    switch (status)
	{
		case OCI_SUCCESS:
			break;

		case OCI_SUCCESS_WITH_INFO:
            throw StoreException(status, "OCI_SUCCESS_WITH_INFO");
                
		case OCI_NEED_DATA:
			throw StoreException(status, "OCI_NEED_DATA");
				
		case OCI_NO_DATA:
			throw StoreException(status, "OCI_NODATA");
                
		case OCI_ERROR:
			if (errhp = conn->errhp)
			{
				(void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
									(sb4 *)&errcode, errbuf,
									(ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
				/*if (errcode == ???)
				{ // Check if connection was dead here !!!
					conn->bIsDead = true;
				}*/
                
                throw StoreException(errcode, (const char *)errbuf);
			}
			break;	
		case OCI_INVALID_HANDLE:
			throw StoreException(status, "OCI_INVALID_HANDLE");
                
		case OCI_STILL_EXECUTING:
			throw StoreException(status, "OCI_STILL_EXECUTE");

        default:
			break;
	}
}

Connection* ConnectionPool::getConnection()
{
    idleLock.Lock();
	connectionsLock.Lock();

	Connection*	connection=0L;
    (void) idle.Pop(connection);
    (void) busy.Push(connection);
    
	if (idle.Count()) idleLock.Unlock();
    connectionsLock.Unlock();
	return connection;
}

void ConnectionPool::freeConnection(Connection* connection)
{
    connectionsLock.Lock();
	Connection*	tmp=0L;
    for (int i=0; i<busy.Count(); i++)
	{
        tmp = busy[i];
		if (tmp == connection) {
			busy.Delete(i);
			if (idle.Count() == 0) idleLock.Unlock();
			(void) idle.Push(connection);
            break;
		}
	}
	connectionsLock.Unlock();
}
/* ----------------------------- ConnectionPool ------------------------ */

/* ------------------------------- Connection -------------------------- */
text* Connection::sqlStoreLock = (text *)
"UPDATE SMS_ID_LOCK SET ID=0 WHERE TGT='SMS_MSG_TABLE'";

text* Connection::sqlStoreMaxId = (text *)
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

Connection::Connection(ConnectionPool* pool, int _id = 0) throw(StoreException) 
	: owner(pool), id(_id), envhp(0L), errhp(0L), svchp(0L), 
		srvhp(0L), sesshp(0L)
{
	rawUd = 0L;

	StoreConfig* config = owner->getConfig();

	const char* userName = config->getUserName();
	const char* userPwd = config->getUserPwd();
	const char* dbName = config->getDbName();
	
	if (userName && userPwd && dbName)
	{
		// open connection to DB and begin user session 
		/*(void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,                         
							 (dvoid * (*)(dvoid *, size_t)) 0,                      
							 (dvoid * (*)(dvoid *, dvoid *, size_t))0,              
							 (void (*)(dvoid *, dvoid *)) 0 );
        
		(void) OCIEnvInit((OCIEnv **) &envhp, OCI_DEFAULT, 
						  (size_t) 0, (dvoid **) 0);*/
		sword status;
		status = OCIEnvCreate(&envhp, OCI_OBJECT|OCI_ENV_NO_MUTEX, 
							  (dvoid *)0, 0, 0, 0, (size_t) 0, (dvoid **)0);
        //printf("Env handle, status = %d\n", status);

		/*dvoid* allocatedMem;
		(void) OCIEnvCreate((OCIEnv **) &envhp, (ub4) OCI_ENV_NO_MUTEX,
							(CONST dvoid *) 0, 
							(CONST dvoid * (*)(dvoid *, size_t)) 0,
							(CONST dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
							(CONST void (*)(dvoid *, dvoid *)) 0, 
							(size_t) 0, (dvoid **) &allocatedMem);*/

		status = OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, 
								OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
		//printf("Err handle, status = %d\n", status);

        checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp,
								OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)); 
		checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp,
								OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0));
        checkErr(OCIServerAttach(srvhp, errhp, (text *)dbName,
								 strlen(dbName), OCI_DEFAULT));
        checkErr(OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX,
							(dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp));
        checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&sesshp,
								OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0));   
        checkErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
							(dvoid *)userName, (ub4)strlen(userName),
							(ub4) OCI_ATTR_USERNAME, errhp));
        checkErr(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
							(dvoid *)userPwd, (ub4) strlen(userPwd),
							(ub4) OCI_ATTR_PASSWORD, errhp));
        checkErr(OCISessionBegin(svchp, errhp, sesshp, OCI_CRED_RDBMS,
								 (ub4) OCI_DEFAULT));
		checkErr(OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
							(dvoid *)sesshp, (ub4) 0,
							(ub4) OCI_ATTR_SESSION, errhp));
		
        // allocate statements handles
		checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtStoreLock,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));
        checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&stmtStoreMaxId,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));
        checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&stmtStoreInsert,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));
        checkErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtRetriveAll,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));

		// prepare statements
		checkErr(OCIStmtPrepare(stmtStoreLock, errhp, sqlStoreLock,
								(ub4)strlen((char *)sqlStoreLock),
								(ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
        checkErr(OCIStmtPrepare(stmtStoreMaxId, errhp, sqlStoreMaxId,
								(ub4)strlen((char *)sqlStoreMaxId),
								(ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
        checkErr(OCIStmtPrepare(stmtStoreInsert, errhp, sqlStoreInsert,
								(ub4)strlen((char *)sqlStoreInsert),
								(ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
        checkErr(OCIStmtPrepare(stmtRetriveAll, errhp, sqlRetriveAll,
								(ub4)strlen((char *)sqlRetriveAll),
								(ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
        
		// define placeholder for max(id)
        checkErr(OCIDefineByPos(stmtStoreMaxId, &defhp, errhp, (ub4) 1,
								(dvoid *) &smsId, (sword) sizeof(smsId),
								SQLT_INT, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
        		
		// bind sms placeholder fields for storing
        checkErr(OCIBindByPos(stmtStoreInsert, &bndStoreId, errhp, (ub4) 1,
							  (dvoid *) &smsId, (sb4) sizeof(smsId),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
		checkErr(OCIBindByPos(stmtStoreInsert, &bndSt, errhp, (ub4) 2,
							  (dvoid *) &(uState), 
							  (sb4) sizeof(uState), SQLT_UIN, 
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgRef, errhp, (ub4) 3,
							  (dvoid *) &(sms.messageReference),
							  (sb4) sizeof(sms.messageReference), SQLT_UIN,
							  (dvoid *)0,(ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgInd, errhp, (ub4) 4,
							  (dvoid *) &(sms.messageIdentifier),
							  (sb4) sizeof(sms.messageIdentifier), SQLT_UIN,
							  (dvoid *)0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOALen, errhp, (ub4) 5,
							  (dvoid *) &(sms.originatingAddress.lenght),
							  (sb4) sizeof(sms.originatingAddress.lenght),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOATon, errhp, (ub4) 6,
							  (dvoid *) &(sms.originatingAddress.type),
							  (sb4) sizeof(sms.originatingAddress.type),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOANpi, errhp, (ub4) 7,
                              (dvoid *) &(sms.originatingAddress.plan),
							  (sb4) sizeof(sms.originatingAddress.plan),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOAVal, errhp, (ub4) 8,
							  (dvoid *)(sms.originatingAddress.value),
							  (sb4) sizeof(AddressValue),
							  SQLT_CHR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDALen, errhp, (ub4) 9,
							  (dvoid *) &(sms.destinationAddress.lenght),
							  (sb4) sizeof(sms.destinationAddress.lenght),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDATon, errhp, (ub4) 10,
							  (dvoid *) &(sms.destinationAddress.type),
							  (sb4) sizeof(sms.destinationAddress.type),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDANpi, errhp, (ub4) 11,
							  (dvoid *) &(sms.destinationAddress.plan),
							  (sb4) sizeof(sms.destinationAddress.plan),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDAVal, errhp, (ub4) 12,
							  (dvoid *)(sms.destinationAddress.value),
							  (sb4) sizeof(AddressValue),
							  SQLT_CHR, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndVTime, errhp, (ub4) 13,
							  (dvoid *)(&validTime), (sb4) sizeof(validTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndWTime, errhp, (ub4) 14,
							  (dvoid *)(&waitTime), (sb4) sizeof(waitTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndSTime, errhp, (ub4) 15,
                              (dvoid *)(&submitTime), (sb4) sizeof(submitTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDTime, errhp, (ub4) 16,
							  (dvoid *)(&deliveryTime), 
							  (sb4) sizeof(deliveryTime), 
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndSrr, errhp, (ub4) 17,
							  (dvoid *) &(bStatusReport),
							  (sb4) sizeof(bStatusReport), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndRd, errhp, (ub4) 18,
							  (dvoid *) &(bRejectDuplicates),
							  (sb4) sizeof(bRejectDuplicates), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgPri, errhp, (ub4) 19,
							  (dvoid *) &(sms.priority), 
							  (sb4) sizeof(sms.priority), SQLT_UIN,
							  (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgPid, errhp, (ub4) 20,
							  (dvoid *) &(sms.protocolIdentifier),
							  (sb4) sizeof(sms.protocolIdentifier),
							  SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndFcs, errhp, (ub4) 21,
							  (dvoid *) &(sms.failureCause),
							  (sb4) sizeof(sms.failureCause), SQLT_UIN,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDcs, errhp, (ub4) 22,
							  (dvoid *) &(sms.messageBody.scheme),
							  (sb4) sizeof(sms.messageBody.scheme),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUdhi, errhp, (ub4) 23,
							  (dvoid *) &(bHeaderIndicator),
							  (sb4) sizeof(bHeaderIndicator), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUdl, errhp, (ub4) 24,
							  (dvoid *) &(sms.messageBody.lenght), 
							  (sb4) sizeof(sms.messageBody.lenght), SQLT_UIN,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUd, errhp, (ub4) 25,
							  (dvoid *) &(rawUd), (sb4) sizeof(rawUd), 
							  SQLT_BIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
		// define placeholders fields for retriving
        checkErr(OCIDefineByPos(stmtRetriveAll, &defSt, errhp, (ub4) 1,
								(dvoid *) &(uState), 
								(sword) sizeof(uState), SQLT_UIN,
								(dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
		checkErr(OCIDefineByPos(stmtRetriveAll, &defMsgRef, errhp, (ub4) 2,
								(dvoid *) &(sms.messageReference), 
								(sword) sizeof(sms.messageReference), SQLT_UIN,
								(dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defMsgInd, errhp, (ub4) 3,
								(dvoid *) &(sms.messageIdentifier), 
								(sword) sizeof(sms.messageIdentifier), SQLT_UIN,
								(dvoid *) 0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defOALen, errhp, (ub4) 4,
								(dvoid *) &(sms.originatingAddress.lenght), 
								(sword) sizeof(sms.originatingAddress.lenght),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defOATon, errhp, (ub4) 5,
								(dvoid *) &(sms.originatingAddress.type), 
								(sword) sizeof(sms.originatingAddress.type),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defOANpi, errhp, (ub4) 6,
								(dvoid *) &(sms.originatingAddress.plan), 
								(sword) sizeof(sms.originatingAddress.plan),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defOAVal, errhp, (ub4) 7,
								(dvoid *) &(sms.originatingAddress.value), 
								(sword) sizeof(sms.originatingAddress.value),
								SQLT_CHR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDALen, errhp, (ub4) 8,
								(dvoid *) &(sms.destinationAddress.lenght), 
								(sword) sizeof(sms.destinationAddress.lenght),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDATon, errhp, (ub4) 9,
								(dvoid *) &(sms.destinationAddress.type), 
								(sword) sizeof(sms.destinationAddress.type),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDANpi, errhp, (ub4) 10,
								(dvoid *) &(sms.destinationAddress.plan), 
								(sword) sizeof(sms.destinationAddress.plan),
								SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDAVal, errhp, (ub4) 11,
								(dvoid *) &(sms.destinationAddress.value), 
								(sword) sizeof(sms.destinationAddress.value),
								SQLT_CHR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defVTime, errhp, (ub4) 12,
								(dvoid *) &(validTime), 
								(sword) sizeof(validTime), SQLT_ODT,
								(dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defWTime, errhp, (ub4) 13,
								(dvoid *) &(waitTime), 
								(sb4) sizeof(waitTime), SQLT_ODT, 
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defSTime, errhp, (ub4) 14,
								(dvoid *) &(submitTime), 
								(sb4) sizeof(submitTime), SQLT_ODT,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDTime, errhp, (ub4) 15,
								(dvoid *) &(deliveryTime), 
								(sb4) sizeof(deliveryTime), SQLT_ODT,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defSrr, errhp, (ub4) 16,
								(dvoid *) &(bStatusReport), 
								(sb4) sizeof(bStatusReport), SQLT_AFC,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defRd, errhp, (ub4) 17,
								(dvoid *) &(bRejectDuplicates), 
								(sb4) sizeof(bRejectDuplicates), SQLT_AFC,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defMsgPri, errhp, (ub4) 18,
								(dvoid *) &(sms.priority), 
								(sb4) sizeof(sms.priority), SQLT_UIN,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defMsgPid, errhp, (ub4) 19,
								(dvoid *) &(sms.protocolIdentifier), 
								(sb4) sizeof(sms.protocolIdentifier), SQLT_UIN,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defFcs, errhp, (ub4) 20,
								(dvoid *) &(sms.failureCause), 
								(sb4) sizeof(sms.failureCause), SQLT_UIN,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
        
        checkErr(OCIDefineByPos(stmtRetriveAll, &defDcs, errhp, (ub4) 21,
								(dvoid *) &(sms.messageBody.scheme), 
								(sb4) sizeof(sms.messageBody.scheme), SQLT_UIN,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defUdhi, errhp, (ub4) 22,
								(dvoid *) &(bHeaderIndicator), 
								(sb4) sizeof(bHeaderIndicator), SQLT_AFC,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defUdl, errhp, (ub4) 23,
								(dvoid *) &(sms.messageBody.lenght), 
								(sb4) sizeof(sms.messageBody.lenght), SQLT_UIN,
								(dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT));
		
        checkErr(OCIDefineByPos(stmtRetriveAll, &defUd, errhp, (ub4) 24,
								(dvoid *) &(rawUd), (sb4) sizeof(rawUd), 
								SQLT_BIN, (dvoid *)0, (ub2 *)0, 
								(ub2 *)0, OCI_DEFAULT));

        checkErr(OCIBindByPos(stmtRetriveAll, &bndRetriveId, errhp, (ub4) 1,
							  (dvoid *) &(smsId), (sb4) sizeof(smsId), 
							  SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
	} 
	else {
		throw AuthenticationException();
	}

}

Connection::~Connection()
{
	if (envhp && errhp && svchp)
    {
        // logoff from database server
        (void) OCILogoff(svchp, errhp);
        
		// free envirounment handle (other handles will be freed too)
        (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
	}
}

void Connection::setSMS(SMS& _sms)
	throw(StoreException)
{
	sms = _sms;
    
	// set additional data
	tm*	dt;

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
	
	checkErr(OCIRawAssignBytes(envhp, errhp, (ub1 *)(sms.messageBody.data),
					  (ub4) sizeof(sms.messageBody.data), &rawUd));

	bStatusReport = sms.statusReportRequested ? 'Y':'N';
	bRejectDuplicates = sms.rejectDuplicates ? 'Y':'N';
	bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
	uState = (uint8_t) sms.state;
}

SMS& Connection::getSMS()
	throw(StoreException)
{
	// get additional data
	sms.state = (State) uState;
    sms.statusReportRequested = (bStatusReport == 'Y');
	sms.rejectDuplicates = (bRejectDuplicates == 'Y');
	sms.messageBody.header = (bHeaderIndicator == 'Y');
    
	ub4 sz = OCIRawSize(envhp, rawUd);
    ub1* buff = OCIRawPtr(envhp, rawUd);
	if (sz <= sizeof(sms.messageBody.data)) {
		memcpy((void *)buff, (void *)sms.messageBody.data, sz);
	}
    
	tm	dt;
	sb2	year;
	ub1	mon, mday, hour, min, sec;

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

    return sms;
}

SMSId Connection::store(SMS& sms) throw(StoreException)
{
	MutexGuard	guard(mutex);

	setSMS(sms);
	
	// lock table
    checkErr(OCIStmtExecute(svchp, stmtStoreLock, errhp, (ub4) 1, (ub4) 0, 
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
    
	// "select max(id) from sms_msg"
	checkErr(OCIStmtExecute(svchp, stmtStoreMaxId, errhp, (ub4) 1, (ub4) 0,
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
    
	smsId++; // If no data present in table smsId = 0 (NVL used)
    
	// insert new sms row into table
	checkErr(OCIStmtExecute(svchp, stmtStoreInsert, errhp, (ub4) 1, (ub4) 0,
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
	
	checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));

	return smsId;
}

SMS& Connection::retrive(SMSId id) throw(StoreException)
{
	MutexGuard	guard(mutex);

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
	// ???
    return getSMS();
}

void Connection::checkErr(sword status) throw(StoreException)
{
	if (owner) 
	{
        owner->checkErr(status, this);
	}
}

/* ------------------------------- Connection -------------------------- */

}}

