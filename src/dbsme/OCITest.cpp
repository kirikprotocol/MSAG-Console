
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include <oci.h>
#include <orl.h>

/*
DROP TABLE SMS_TEST;
CREATE TABLE SMS_TEST
(
        TEMP            NUMBER          NULL,
        STR             VARCHAR2(256)   NULL,
        DT              DATE            NULL
);
*/

static const char* DB_INSTANCE  = "SMSC";
static const char* DB_USERNAME  = "SMSC_MAKAR";
static const char* DB_USERPWD   = "SMSC_MAKAR";
static const char* SQL_RS_QUERY = "SELECT * FROM SMS_TEST";

bool check(OCIError *errhp, sword status)
{
  text errbuf[512];
  sb4 errcode = 0;

  switch (status)
  {
  case OCI_SUCCESS:
    return true;
  case OCI_SUCCESS_WITH_INFO:
    (void) printf("Error - OCI_SUCCESS_WITH_INFO\n");
    return true;
  case OCI_NEED_DATA:
    (void) printf("Error - OCI_NEED_DATA\n");
    break;
  case OCI_NO_DATA:
    (void) printf("Error - OCI_NODATA\n");
    break;
  case OCI_ERROR:
    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
                        errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
    (void) printf("Error - %.*s\n", 512, errbuf);
    break;
  case OCI_INVALID_HANDLE:
    (void) printf("Error - OCI_INVALID_HANDLE\n");
    break;
  case OCI_STILL_EXECUTING:
    (void) printf("Error - OCI_STILL_EXECUTE\n");
    break;
  case OCI_CONTINUE:
    (void) printf("Error - OCI_CONTINUE\n");
    break;
  default:
    break;
  }
  return false;
}

int main(char *argv, int argc)
{
  /* Declare OCI Handles to be used */
  OCIEnv        *envhp = 0;
  OCIServer     *srvhp = 0; 
  OCISvcCtx     *svchp = 0;
  OCIError      *errhp = 0;
  OCISession    *authp = 0;
  
  /* Create and Initialize an OCI Environment: */
  (void) OCIEnvCreate(&envhp, (ub4)OCI_OBJECT|OCI_ENV_NO_MUTEX /*!OCI_DEFAULT*/,
                      (dvoid *)0, (dvoid * (*)(dvoid *, size_t)) 0,
                      (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                      (void (*)(dvoid *, dvoid *))0,
                      (size_t) 0, (dvoid **) 0);
  
  /* Allocate error handle, server context, service context: */
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, 
                        (size_t) 0, (dvoid **) 0);
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
                        (size_t) 0, (dvoid **) 0);
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
                        (size_t) 0, (dvoid **) 0);

  /* Attach to the Oracle database:  */
  (void) OCIServerAttach(srvhp, errhp, (text *)DB_INSTANCE, strlen(DB_INSTANCE), OCI_DEFAULT);

  /* Set the server context attribute in the service context: */
  (void) OCIAttrSet ((dvoid *) svchp, OCI_HTYPE_SVCCTX, 
                     (dvoid *)srvhp, (ub4) 0,
                     OCI_ATTR_SERVER, (OCIError *) errhp);
  /* Allocate the session handle: */
  (void) OCIHandleAlloc((dvoid *) envhp, 
                        (dvoid **)&authp, (ub4) OCI_HTYPE_SESSION,
                        (size_t) 0, (dvoid **) 0);
  
  /* Set username & password in the session handle:*/
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                    (dvoid *) DB_USERNAME, (ub4)strlen(DB_USERNAME),
                    (ub4) OCI_ATTR_USERNAME, errhp);
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                    (dvoid *) DB_USERPWD, (ub4)strlen(DB_USERPWD),
                    (ub4) OCI_ATTR_PASSWORD, errhp);

  /* Authenticate and begin the session: */
  check(errhp, OCISessionBegin (svchp,  errhp, authp, OCI_CRED_RDBMS, 
                                   (ub4) OCI_DEFAULT));
  /* Set the session attribute in the service context: */
  (void) OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                    (dvoid *) authp, (ub4) 0, (ub4) OCI_ATTR_SESSION, errhp);

  /* ------- At this point a valid session has been created -----------*/
  printf ("User session created. Connection etablished. \n");

  OCIStmt *stmt = 0;
  check(errhp, OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &stmt,
                               OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0));
  check(errhp, OCIStmtPrepare(stmt, errhp, (text *)SQL_RS_QUERY, (ub4) strlen(SQL_RS_QUERY),
                              (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));

  check(errhp, OCIStmtExecute(svchp, stmt, errhp, 0, 0, (CONST OCISnapshot *) NULL,
                              (OCISnapshot *) NULL, OCI_DEFAULT));
  OCIParam *param = 0;
  ub4 counter = 1;

  /* Request a parameter descriptor for position 1 in the select-list */
  sb4 status = OCIParamGet(stmt, OCI_HTYPE_STMT, errhp, (dvoid **)&param, (ub4) counter);
  
  /* Loop only if a descriptor was successfully retrieved for
  current position, starting at 1 */
  while (status == OCI_SUCCESS)
  {
      ub2 type = 0; ub2 size = 0;

      /* Retrieve the data type attribute */
      check(errhp, OCIAttrGet((dvoid *) param, (ub4) OCI_DTYPE_PARAM,
                              (dvoid *) &type, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
                              (OCIError*) errhp));
      /* Retrieve max data size */
      check(errhp, OCIAttrGet((dvoid *) param, (ub4) OCI_DTYPE_PARAM,
                              (dvoid *) &size, (ub4 *) 0, (ub4) OCI_ATTR_DATA_SIZE,
                              (OCIError *) errhp));

      printf("Parameter %lu: type %d size %ld\n", counter, type, size);

      check(errhp, OCIDescriptorFree((dvoid *) param, OCI_DTYPE_PARAM));
      /* increment counter and get next descriptor, if there is one */
      status = OCIParamGet(stmt, OCI_HTYPE_STMT, errhp, (dvoid **)&param, (ub4) ++counter);
  }
  
  // free statement
  if (stmt) (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
  // logoff from database server
  if (errhp && svchp) (void) OCILogoff(svchp, errhp); 
  // free envirounment handle, all derrived handles will be freed too
  if (envhp) (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);

  return 0;
}


