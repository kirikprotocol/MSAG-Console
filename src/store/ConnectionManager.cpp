#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oci.h>
#include <orl.h>

#include <util/debug.h>
#include <sms/sms.h>
#include "ConnectionManager.h"

namespace smsc { namespace store 
{
using namespace smsc::sms;

/* ----------------------------- ConnectionPool ------------------------ */
ConnectionPool::ConnectionPool(StoreConfig* _config) 
	throw(ConnectionFailedException)
		: config(_config)
{
    __require__(config);
	
	maxConnectionsCount = config->getMaxConnectionsCount();
	curConnectionsCount = config->getInitConnectionsCount();
	
	__require__(curConnectionsCount > 0);
	__require__(curConnectionsCount <= maxConnectionsCount);

    for (int i=0; i<curConnectionsCount; i++)
	{
		Connection*	connection = new Connection(this);
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
}

void ConnectionPool::checkErr(sword status, Connection* connection) 
	throw(StorageException)
{
    OCIError*	errhp;
	text		errbuf[512];
    ub4			buflen, errcode;
    
	__require__(connection);
    
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
        if (errhp = connection->errhp)
		{
			(void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
								(sb4 *)&errcode, errbuf,
								(ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
			status = errcode;
        }
		break;	

	default:
		strcpy((char *)errbuf, "OCI_UNKNOWN_ERROR");
		break;
	}
	
	connectionsLock.Lock();
    Connection*	tmp=0L;
    for (int i=0; i<busy.Count(); i++)
	{
        tmp = busy[i];
		if (tmp == connection) // set Connection dead
		{ 
			busy.Delete(i); 
            (void) dead.Push(connection);
			curConnectionsCount--;
            break;
		}
	}
    connectionsLock.Unlock();

    throw StorageException((const char *)errbuf, (int)status);
}

Connection* ConnectionPool::getConnection()
	throw(ConnectionFailedException)
{
    idleLock.Lock();
	connectionsLock.Lock();
	
	Connection*	connection=0L;
	
	if (!idle.Count())
	{
		__require__(curConnectionsCount < maxConnectionsCount);

        try 
		{
			connection = new Connection(this);
		}
		catch (ConnectionFailedException& exc) 
		{
			idleLock.Unlock();
			connectionsLock.Unlock();
			throw;
		}
		(void) busy.Push(connection);
        if (++curConnectionsCount < maxConnectionsCount)
		{
			idleLock.Unlock();
		}
    } 
	else 
	{
		(void) idle.Pop(connection);
		(void) busy.Push(connection);
		if (idle.Count() || curConnectionsCount < maxConnectionsCount)
		{
			idleLock.Unlock();
		}
    }

    connectionsLock.Unlock();
	return connection;
}

// from busy --> idle
void ConnectionPool::freeConnection(Connection* connection)
{
    connectionsLock.Lock();
	
	__require__(connection);

	Connection*	tmp=0L;
	if (dead.Count())
	{
		for (int i=0; i<dead.Count(); i++)
		{
			tmp = dead[i];
			if (tmp == connection) 
			{
				dead.Delete(i);
				delete connection;
                break;
			}
		}
	}
	else 
	{
		for (int i=0; i<busy.Count(); i++)
		{
			tmp = busy[i];
			if (tmp == connection) 
			{
				busy.Delete(i);
				(void) idle.Push(connection);
				if (idle.Count() == 1) idleLock.Unlock();
				break;
			}
		}
    }
	
    connectionsLock.Unlock();
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

Connection::Connection(ConnectionPool* pool) 
	throw(ConnectionFailedException) 
		: owner(pool), envhp(0L), errhp(0L), svchp(0L), srvhp(0L), sesshp(0L)
{
    StoreConfig* config = owner->getConfig();

	__require__(config);

	const char* dbName = config->getDBInstance();
	const char* userName = config->getDBUserName();
	const char* userPwd = config->getDBUserPassword();
	
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

	/*
	// initialize the context by 'this'
	failover.fo_ctx = (dvoid *)this;
	failover.callback_function = 
		&(smsc::store::Connection::_failoverCallback);
	// do the registration
	checkConnErr(OCIAttrSet(srvhp, (ub4) OCI_HTYPE_SERVER, (dvoid *) &failover, 
							(ub4) 0, (ub4) OCI_ATTR_FOCBK, errhp));
	*/

	checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtGetMessagesCount,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));
	checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtStoreInsert,
								OCI_HTYPE_STMT, 0, (dvoid **) 0));
	checkConnErr(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmtRetriveAll,
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
							  (dvoid *) &sms.messageBody.data,
							  (sb4) sizeof(sms.messageBody.data), 
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
								(dvoid *) sms.messageBody.data, 
								(sb4) sizeof(sms.messageBody.data), 
								SQLT_BIN, (dvoid *)0, (ub2 *)0, 
								(ub2 *)0, OCI_DEFAULT));

	checkConnErr(OCIBindByPos(stmtRetriveAll, &bndRetriveId, errhp, (ub4) 1,
							  (dvoid *) &(smsId), (sb4) sizeof(smsId), 
							  SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
}

Connection::~Connection()
{
	__require__(envhp && errhp && svchp);
    
	// logoff from database server
	(void) OCILogoff(svchp, errhp);

	// free envirounment handle (other handles will be freed too)
	(void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
}

SMSId Connection::getMessagesCount()
	throw(ConnectionFailedException)
{
	MutexGuard	guard(mutex);

	checkConnErr(OCIStmtExecute(svchp, stmtGetMessagesCount, errhp, 
								(ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL,
								(OCISnapshot *) NULL, OCI_DEFAULT));
	return smsId;
}

void Connection::setSMS(const SMS &_sms)
	throw(StorageException)
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
	
	bStatusReport = sms.statusReportRequested ? 'Y':'N';
	bRejectDuplicates = sms.rejectDuplicates ? 'Y':'N';
	bHeaderIndicator = sms.messageBody.header ? 'Y':'N';
	uState = (uint8_t) sms.state;
}

void Connection::store(const SMS &sms, SMSId id) 
	throw(StorageException)
{
	MutexGuard	guard(mutex);

	smsId = id;
    
	try 
	{
        setSMS(sms);
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

const SMS& Connection::getSMS()
	throw(StorageException)
{
	// get additional data
	sms.state = (State) uState;
    sms.statusReportRequested = (bStatusReport == 'Y');
	sms.rejectDuplicates = (bRejectDuplicates == 'Y');
	sms.messageBody.header = (bHeaderIndicator == 'Y');
    
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

const SMS& Connection::retrive(SMSId id) 
	throw(StorageException, NoSuchMessageException)
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
	checkErr(status);
    
	return getSMS();
}

sb4 Connection::failoverCallback(
	dvoid *svchp, dvoid *envhp, dvoid *fo_ctx, ub4 fo_type, ub4 fo_event)
{
	printf("Failover ocured !!!\n");
	return 0;
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
	__require__(owner);
    
    owner->checkErr(status, this);
}

/* ------------------------------- Connection -------------------------- */

}}

