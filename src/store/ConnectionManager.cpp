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
	: config(_config), conn(0L), lock(false)
{

	(void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,                         
						 (dvoid * (*)(dvoid *, size_t)) 0,                      
						 (dvoid * (*)(dvoid *, dvoid *, size_t))0,              
						 (void (*)(dvoid *, dvoid *)) 0 );                      
    //create mutex here
    if (!conn) 
    {
        if (!config) {
            throw AuthenticationException();
        }
        conn = new Connection(this);
    }
}

ConnectionPool::~ConnectionPool()
{
	if (config) delete config; 
	
    if (conn)
    {
        delete conn; conn = 0L;
	}
	// destory mutex here
}

void ConnectionPool::checkErr(sword status, Connection* conn) throw(StoreException)
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
				
					// Check if connection was dead here !!!

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
    while (lock); // replace it by mutex logic (own mutex here)
    lock = true;
    return conn;
}

void ConnectionPool::freeConnection(Connection* connection)
{
    // replace it by mutex logic (free mutex here)
    if (conn == connection) 
	{
        lock = false;
    }
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
 SRR, RD, PRI, PID, FCS, DCS, UDHI, UDL, UD\
 FORM SMS_MSG WHERE ID=:ID";

Connection::Connection(ConnectionPool* pool, int _id = 0) throw(StoreException) 
	: owner(pool), id(_id), envhp(0L), errhp(0L), svchp(0L), 
		srvhp(0L), sesshp(0L)
{
	StoreConfig* config = owner->getConfig();

	const char* userName = config->getUserName();
	const char* userPwd = config->getUserPwd();
	const char* dbName = config->getDbName();
	
	if (userName && userPwd && dbName)
	{
        // open connection to DB and begin user session 
		(void) OCIEnvInit((OCIEnv **) &envhp, OCI_DEFAULT, 
						  (size_t) 0, (dvoid **) 0);
		(void) OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp, 
								OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
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
        OCIDefine*  defhp;
        checkErr(OCIDefineByPos(stmtStoreMaxId, &defhp, errhp, (ub4) 1,
								(dvoid *) &smsId, (sword) sizeof(smsId),
								SQLT_INT, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));
        		
		// bind sms placeholder fields to insert statement
        OCIBind*    bndId;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndId, errhp, (ub4) 1,
							  (dvoid *) &smsId, (sb4) sizeof(smsId),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndState;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndState, errhp, (ub4) 2,
							  (dvoid *) &(sms.state), 
							  (sb4) sizeof(sms.state), SQLT_UIN, 
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndMsgRef;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgRef, errhp, (ub4) 3,
							  (dvoid *) &(sms.messageReference),
							  (sb4)sizeof(sms.messageReference), SQLT_UIN,
							  (dvoid *)0,(ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndMsgIdent;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgIdent, errhp, (ub4) 4,
							  (dvoid *) &(sms.messageIdentifier),
							  (sb4)sizeof(sms.messageIdentifier), SQLT_UIN,
							  (dvoid *)0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndOALen;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOALen, errhp, (ub4) 5,
							  (dvoid *) &(sms.originatingAddress.lenght),
							  (sb4) sizeof(sms.originatingAddress.lenght),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndOATon;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOATon, errhp, (ub4) 6,
							  (dvoid *) &(sms.originatingAddress.type),
							  (sb4) sizeof(sms.originatingAddress.type),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        OCIBind*    bndOANpi;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOANpi, errhp, (ub4) 7,
                              (dvoid *) &(sms.originatingAddress.plan),
							  (sb4) sizeof(sms.originatingAddress.plan),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndOAVal;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndOAVal, errhp, (ub4) 8,
							  (dvoid *)(sms.originatingAddress.value),
							  (sb4)sizeof(sms.originatingAddress.value),
							  SQLT_CHR, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndDALen;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDALen, errhp, (ub4) 9,
							  (dvoid *) &(sms.destinationAddress.lenght),
							  (sb4) sizeof(sms.destinationAddress.lenght),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndDATon;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDATon, errhp, (ub4) 10,
							  (dvoid *) &(sms.destinationAddress.type),
							  (sb4) sizeof(sms.destinationAddress.type),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        OCIBind*    bndDANpi;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDANpi, errhp, (ub4) 11,
							  (dvoid *) &(sms.destinationAddress.plan),
							  (sb4) sizeof(sms.destinationAddress.plan),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        OCIBind*    bndDAVal;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDAVal, errhp, (ub4) 12,
							  (dvoid *)(sms.destinationAddress.value),
							  (sb4)sizeof(sms.destinationAddress.value),
							  SQLT_CHR, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndVTime;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndVTime, errhp, (ub4) 13,
							  (dvoid *)(&validTime), (sb4)sizeof(validTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndWTime;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndWTime, errhp, (ub4) 14,
							  (dvoid *)(&waitTime), (sb4)sizeof(waitTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndSTime;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndSTime, errhp, (ub4) 15,
                              (dvoid *)(&submitTime), (sb4)sizeof(submitTime),
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndDTime;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDTime, errhp, (ub4) 16,
							  (dvoid *)(&deliveryTime), 
							  (sb4)sizeof(deliveryTime), 
							  SQLT_ODT, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT)); 
        OCIBind*    bndSrr;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndSrr, errhp, (ub4) 17,
							  (dvoid *) &(bStatusReport),
							  (sb4) sizeof(bStatusReport), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndRd;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndRd, errhp, (ub4) 18,
							  (dvoid *) &(bRejectDuplicates),
							  (sb4) sizeof(bRejectDuplicates), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndMsgPri;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgPri, errhp, (ub4) 19,
							  (dvoid *) &(sms.priority), 
							  (sb4)sizeof(sms.priority), SQLT_UIN,
							  (dvoid *)0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndMsgPid;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndMsgPid, errhp, (ub4) 20,
							  (dvoid *) &(sms.protocolIdentifier),
							  (sb4)sizeof(sms.protocolIdentifier),
							  SQLT_UIN, (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndFcs;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndFcs, errhp, (ub4) 21,
							  (dvoid *) &(sms.failureCause),
							  (sb4) sizeof(sms.failureCause), SQLT_UIN,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndDcs;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndDcs, errhp, (ub4) 22,
							  (dvoid *) &(sms.messageBody.scheme),
							  (sb4) sizeof(sms.messageBody.scheme),
							  SQLT_UIN, (dvoid *) 0, (ub2 *)0, (ub2 *)0, 
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        OCIBind*    bndUdhi;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUdhi, errhp, (ub4) 23,
							  (dvoid *) &(bHeaderIndicator),
							  (sb4) sizeof(bHeaderIndicator), SQLT_AFC,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        OCIBind*    bndUdl;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUdl, errhp, (ub4) 24,
							  (dvoid *) &(sms.messageBody.lenght), 
							  (sb4) sizeof(sms.messageBody.lenght), SQLT_UIN,
							  (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4)0,
							  (ub4 *)0, OCI_DEFAULT));
        
        OCIBind*    bndUd;
        checkErr(OCIBindByPos(stmtStoreInsert, &bndUd, errhp, (ub4) 25,
							  (dvoid *)(sms.messageBody.data), 
							  (sb4)sizeof(sms.messageBody.data), SQLT_CHR,
							  (dvoid *)0, (ub2 *)0, (ub2 *)0,
							  (ub4)0, (ub4 *)0, OCI_DEFAULT));
        
		/*OCIDefine*  defSt;
        checkErr(OCIDefineByPos(stmtStoreMaxId, &defhp, errhp, (ub4) 1,
								(dvoid *) &smsId, (sword) sizeof(smsId),
								SQLT_INT, (dvoid *) 0, (ub2 *)0, (ub2 *)0,
								OCI_DEFAULT));*/
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
{
	sms = _sms;
    
	// set additional data
	tm*         dt;

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
	
	bStatusReport = sms.isStatusReportRequested() ? 'Y':'N';
	bRejectDuplicates = sms.isRejectDuplicates() ? 'Y':'N';
	bHeaderIndicator = sms.messageBody.isHeaderIndicator() ? 'Y':'N';
}

SMS& Connection::getSMS()
{
	// get additional data here !!!
	return sms;
}

SMSId Connection::store(SMS& sms) throw(StoreException)
{
	setSMS(sms);
	
	// lock table
    checkErr(OCIStmtExecute(svchp, stmtStoreLock, errhp, (ub4) 1, (ub4) 0, 
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
	
    // "select max(id) from sms_msg"
	checkErr(OCIStmtExecute(svchp, stmtStoreMaxId, errhp, (ub4) 1, (ub4) 0,
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
    
	// !!!! check if no_data here (empty table) !!!!
	
	smsId++;
	
	// insert new sms row into table
	checkErr(OCIStmtExecute(svchp, stmtStoreInsert, errhp, (ub4) 1, (ub4) 0,
							(CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
							OCI_DEFAULT));
	
	checkErr(OCITransCommit(svchp, errhp, OCI_DEFAULT));

	return smsId;
}

SMS& Connection::retrive(SMSId id) throw(StoreException)
{
	
	return getSMS();
}

void Connection::checkErr(sword status) throw(StoreException)
{
	if (owner) 
	{
		try {
			owner->checkErr(status, this);
		}
		catch (StoreException& exc) {
			//(void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
			throw;
		}
	}
}

/* ------------------------------- Connection -------------------------- */

}}

