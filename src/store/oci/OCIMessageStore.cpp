#include <stdio.h> // remove it later
#include <stdlib.h>
#include <string.h>

#include "OCIMessageStore.h"

namespace smsc { namespace store { namespace oci 
{

    using namespace smsc::store;
    
/* --------------- OCIMessageStore ----------------- */
OCIMessageStore::OCIMessageStore(OCIStoreConfig* _config)
    : MessageStore(_config), isOpened(false),
	envhp(0L), errhp(0L), svchp(0L)
{

}

OCIMessageStore::~OCIMessageStore()
{
    close();
}

void OCIMessageStore::checkerror(OCIError *errhp, sword status)
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
 
void OCIMessageStore::open()
    throw(ResourceAllocationException, AuthenticationException)
{
    if (!isOpened)
    {
        if (!config) throw (AuthenticationException());
        
        const char* userName = config->getUserName();
        const char* userPwd = config->getUserPwd();
        const char* dbName = config->getDbName();
        
        if (userName && userPwd && dbName)
        {
            sword       status;

            /* initialize the mode to be the threaded environment */
            status = OCIEnvCreate(&envhp, OCI_THREADED, (dvoid *)0,
                                   0, 0, 0, (size_t) 0, (dvoid **)0);
            if (status != OCI_SUCCESS || !envhp) {
                throw ResourceAllocationException();
            }
            
            /* allocate an error handle */
            status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&errhp,
                                    OCI_HTYPE_ERROR, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !errhp) {
                throw ResourceAllocationException();
            }
	    
	    /*
	    // create a server context
	    (void) OCIServerAttach (srvhp, errhp, (text *)dbName,
			    strlen (dbName), OCI_DEFAULT);

	    // allocate a service handle
	    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&svchp,
			    OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !svchp) {
                throw ResourceAllocationException();
            }

	    // set the server attribute in the service context handle
	    (void) OCIAttrSet ((dvoid *)svchp, OCI_HTYPE_SVCCTX,
			    (dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp);
    
	    // allocate a user session handle
	    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&usrhp,
			    OCI_HTYPE_SESSION, 0, (dvoid **) 0);
            if (status != OCI_SUCCESS || !usrhp) {
                throw ResourceAllocationException();
            }

	    // set username attribute in user session handle
	    (void) OCIAttrSet ((dvoid *)usrhp, OCI_HTYPE_SESSION,
				(dvoid *)userName, (ub4)strlen(userName),
				OCI_ATTR_USERNAME, errhp);
	    // set password attribute in user session handle
	    (void) OCIAttrSet ((dvoid *)usrhp, OCI_HTYPE_SESSION,
				(dvoid *)userPwd, (ub4)strlen(userPwd),
				OCI_ATTR_PASSWORD, errhp);

	    status = OCISessionBegin ((OCISvcCtx *)svchp, errhp, usrhp,
				    OCI_CRED_RDBMS, OCI_DEFAULT);
            if (status != OCI_SUCCESS) {
                checkerror(errhp, status);
		        (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
		        throw AuthenticationException();
            }
				    
	    // set the user session attribute in the service context handle
	    status = OCIAttrSet ((OCISvcCtx *)svchp, OCI_HTYPE_SVCCTX,
			(dvoid *)usrhp, (ub4) 0, OCI_ATTR_SESSION, errhp);				            
            if (status != OCI_SUCCESS) {
		checkerror(errhp, status);
		(void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
                throw AuthenticationException();
            }*/
			
            // logon to server database (allocate a service handle) 
            status = OCILogon(envhp, errhp, &svchp,
                                (OraText*)userName, strlen(userName),
                                (OraText*)userPwd, strlen(userPwd),
                                (OraText*)dbName, strlen(dbName));
            if (status != OCI_SUCCESS || !svchp)
            {
                // free envirounment handle (error handle will be freed too)
		        checkerror(errhp, status);
                (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
                throw AuthenticationException();
            }
            isOpened = true;
        }
        else throw AuthenticationException();
    }
}

void OCIMessageStore::close()
{
    if (isOpened && envhp && errhp && svchp)
    {
        sword       status;
        
        /* logoff from server */
        (void) OCILogoff(svchp, errhp);
        
        /* free envirounment handle (error & service handles will be freed too) */
        (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
        svchp = 0L; errhp = 0L; envhp = 0L;
        isOpened = false;
    }
}

sms::SMSId OCIMessageStore::store(sms::SMS* message)
    throw(ResourceAllocationException)
{
    sms::SMSId  id = 0; 
    OCIError*   lerrhp;
    sword       status;

    /* allocate an new local error handle */
    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&lerrhp,
                                OCI_HTYPE_ERROR, 0, (dvoid **) 0);
    if (status != OCI_SUCCESS || !lerrhp) {
        throw ResourceAllocationException();
    }
    
    // Code for actual storing follows

    OCIStmt*   stmthp;
    /* allocate an statement handle */
    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&stmthp,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0);
    if (status != OCI_SUCCESS || !stmthp) {
        throw ResourceAllocationException();
    }
    
    /* prepare statement */
    static text* sqlStmt = (text *)
    "INSERT INTO SMS_MSG ID, ST, MR, RM,\
	    OA_LEN, OA_TON, OA_NPI, OA_VAL,\
	    DA_LEN, DA_TON, DA_NPI, DA_VAL,\
	    VALID_TIME, SUBMIT_TIME, DELIVERY_TIME, WAIT_TIME,\
	    PRI, SRR, RD, PID, FCS, DCS, UDHI, UD\
     VALUE (:ID, :ST, :MR, :RM,\
    	    :OA_LEN, :OA_TON, :OA_NPI, :OA_VAL,\
	    :DA_LEN, :DA_TON, :DA_NPI, :DA_VAL,\
	    :PRI, :SRR, :RD, :PID, :FCS, :DCS, :UDHI, :UD)";
    (void) OCIStmtPrepare(stmthp, lerrhp, sqlStmt, (ub4)strlen((char *)sqlStmt), 
                            (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
    
    /* bing placeholders to statement */
    OCIBind*    id_bindp = (OCIBind *) 0;
    (void) OCIBindByName(stmthp, &id_bindp, lerrhp, (text *)":ID", -1,
			 (dvoid *)&id, (sword)sizeof(id), SQLT_INT, (dvoid *)0, 
                         (ub2 *)0, (ub2 *)0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    
    /* execute prepared statement */
    status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0, 
                            (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                             OCI_DEFAULT);

    /* free statement handle*/
    (void) OCIHandleFree(stmthp, OCI_HTYPE_STMT);

    /* free error handle */
    (void) OCIHandleFree(lerrhp, OCI_HTYPE_ERROR);
    return id;
}

sms::SMS* OCIMessageStore::retrive(sms::SMSId id)
    throw(ResourceAllocationException, NoSuchMessageException)
{
    sms::SMS*   sms;
    OCIError*   lerrhp;
    sword       status;
    /* allocate an error handle */
    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&lerrhp,
                                OCI_HTYPE_ERROR, 0, (dvoid **) 0);
    if (status != OCI_SUCCESS || !lerrhp) {
        throw ResourceAllocationException();
    }
    
    // Code for actual retriving follows

    OCIStmt*   stmthp;
    /* allocate an statement handle */
    status = OCIHandleAlloc ((dvoid *)envhp, (dvoid **)&stmthp,
                                OCI_HTYPE_STMT, 0, (dvoid **) 0);
    if (status != OCI_SUCCESS || !stmthp) {
        throw ResourceAllocationException();
    }
    
    /* prepare statement */
    static text* sqlStmt = (text *)"SELECT * FROM SMS_MSG WHERE ID=:id";
    /*"SELECT ST, OA_LEN, OA_TON, OA_NPI, OA_VAL, DA_LEN, DA_TON, DA_NPI, DA_VAL,\
            VALID_TIME, SUBMIT_TIME, DELIVERY_TIME, WAIT_TIME, \
            PRI, SRR, RD, PID, FCS, DCS, UDHI, UD\
     FROM Messages WHERE ID=:id";*/
    (void) OCIStmtPrepare(stmthp, lerrhp, sqlStmt, (ub4)strlen((char *)sqlStmt), 
                            (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
    
    /* bing id to statement */
    OCIBind*    id_bindp = (OCIBind *) 0;
    (void) OCIBindByName(stmthp, &id_bindp, lerrhp, (text *)":id", -1,(dvoid *)&id,
                         (sword)sizeof(id), SQLT_UIN, (dvoid *)0, 
                         (ub2 *)0, (ub2 *)0, (ub4) 0, (ub4 *)0, OCI_DEFAULT);
    
    /* execute prepared statement */
    status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0, 
                            (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                             OCI_DEFAULT);
    if (status == OCI_NO_DATA) 
    {
        /* free statement handle*/
        (void) OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        /* free error handle */
        (void) OCIHandleFree(lerrhp, OCI_HTYPE_ERROR);
        throw NoSuchMessageException();
    }

    /* free statement handle*/
    (void) OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    /* free error handle */
    (void) OCIHandleFree(lerrhp, OCI_HTYPE_ERROR);
    
    return sms;
}
/* --------------- OCIMessageStore ----------------- */

}}}


