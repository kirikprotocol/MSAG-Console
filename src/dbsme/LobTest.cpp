
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include <oci.h>
#include <orl.h>

static const int bodyBufferSize = 1024000;
static char bodyBuffer[bodyBufferSize];

bool checkerr(OCIError *errhp, sword status)
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

/* Select the locator into a locator variable */

bool select_frame_locator(OCILobLocator *Lob_loc, 
                          OCIError *errhp, OCISvcCtx *svchp,
                          OCIStmt *stmthp)
{
  text      *sqlstmt = (text *)"SELECT BODY FROM SMS_ATCH "
                               "WHERE ID='0000000000000001' FOR UPDATE";
  OCIDefine *defnp1;

  if (!checkerr (errhp, OCIStmtPrepare(stmthp, errhp, sqlstmt, 
                                  (ub4)strlen((char *)sqlstmt),
                                  (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCIDefineByPos(stmthp, &defnp1, errhp, (ub4) 1,
                                  (dvoid *)&Lob_loc, (sb4)0, 
                                  (ub2) SQLT_BLOB,(dvoid *) 0, 
                                  (ub2 *) 0, (ub2 *) 0, (ub4) OCI_DEFAULT)))
      return false;

  /* execute the select and fetch one row */
  if (!checkerr (errhp, OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                                 (CONST OCISnapshot*) 0, (OCISnapshot*) 0,  
                                 (ub4) OCI_DEFAULT)))
      return false;
  
  return true;
}

bool processLOB(OCIEnv *envhp, OCIError *errhp,
                OCISvcCtx *svchp, OCIStmt *stmthp)
{
  OCILobLocator *Lob_loc;
  int isOpen = false;

  /* allocate locator resources */
  (void) OCIDescriptorAlloc((dvoid *)envhp, (dvoid **)&Lob_loc,
                            (ub4)OCI_DTYPE_LOB, (size_t)0, (dvoid **)0);

  /* Select the locator */
  if (!select_frame_locator(Lob_loc, errhp, svchp, stmthp))
      return false;
  
  if (!checkerr (errhp, OCILobIsOpen(svchp, errhp, Lob_loc, &isOpen)))
      return false;
  printf("Lob is %sOpen\n", ((isOpen) ? "":"not "));

  if (!checkerr (errhp, OCILobOpen(svchp, errhp, Lob_loc, OCI_LOB_READWRITE)))
      return false;
  
  
  if (!checkerr (errhp, OCILobIsOpen(svchp, errhp, Lob_loc, &isOpen)))
      return false;
  printf("Lob is %sOpen\n", ((isOpen) ? "":"not "));
  
  
  if (!checkerr (errhp, OCILobClose(svchp, errhp, Lob_loc)))
      return false;

  printf("Lob is Closed !\n");
  /* Free resources held by the locators*/
  (void) OCIDescriptorFree((dvoid *) Lob_loc, (ub4) OCI_DTYPE_LOB);
  
  return true;
}

bool writeToDB(OCIEnv *envhp, OCIError *errhp,
                OCISvcCtx *svchp, OCIStmt *stmthp)
{
  uint64_t id = 5;

  OCILobLocator *blobLoc;
  int isOpen = false;
  
  text      *insertSql = (text *)
  "INSERT INTO SMS_ATCH (ID, BODY) VALUES (:ID, EMPTY_BLOB()) "
  "RETURNING BODY INTO :BODY";

  OCIBind *bindId;
  OCIBind *bindBody;

  (void) OCIDescriptorAlloc((dvoid *)envhp, (dvoid **)&blobLoc,
                            (ub4)OCI_DTYPE_LOB, (size_t)0, (dvoid **)0);
  
  if (!checkerr (errhp, OCIStmtPrepare(stmthp, errhp, insertSql, 
                                  (ub4)strlen((char *)insertSql),
                                  (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCIBindByPos(stmthp, &bindId, errhp, (ub4) 1,
                                     (dvoid *)&id, (sb4)sizeof(id), 
                                     (ub2) SQLT_BIN,(dvoid *) 0, 
                                     (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                                     (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCIBindByPos(stmthp, &bindId, errhp, (ub4) 2,
                                     (dvoid *)&blobLoc, 
                                     (sb4)sizeof(OCILobLocator *),
                                     (ub2) SQLT_BLOB,(dvoid *) 0, 
                                     (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                                     (ub4) OCI_DEFAULT)))
      return false;
  
  /* execute the select and fetch one row */
  if (!checkerr (errhp, OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                                 (CONST OCISnapshot*) 0, (OCISnapshot*) 0,  
                                 (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCILobOpen(svchp, errhp, blobLoc, OCI_LOB_READWRITE)))
      return false;
  if (!checkerr (errhp, OCILobIsOpen(svchp, errhp, blobLoc, &isOpen)))
      return false;
  printf("Lob is %sOpen\n", ((isOpen) ? "":"not "));

  ub4 amount = sizeof(bodyBuffer);
  ub4 offset = 1;
  
  printf("Writing to LOB ...\nBytes to write: %d\n", amount);
  if (!checkerr (errhp, OCILobWrite(svchp, errhp, blobLoc, &amount, offset, 
                                    (dvoid *)bodyBuffer, sizeof(bodyBuffer), 
                                    OCI_ONE_PIECE, (dvoid *)0,
                                    (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 *)) 0,
                                    (ub2) 0, (ub1) 0)))
      return false;
  printf("Bytes written: %d\n", amount);
  
  if (!checkerr (errhp, OCILobClose(svchp, errhp, blobLoc)))
      return false;
  printf("Lob is Closed !\n");

  /* Free resources held by the locators*/
  (void) OCIDescriptorFree((dvoid *) blobLoc, (ub4) OCI_DTYPE_LOB);
  return (checkerr (errhp, OCITransCommit(svchp, errhp, OCI_DEFAULT)));
}

bool readFromDB(OCIEnv *envhp, OCIError *errhp,
                OCISvcCtx *svchp, OCIStmt *stmthp)
{
  uint64_t id = 5;

  OCILobLocator *blobLoc;
  int isOpen = false;
  
  text      *selectSql = (text *)
  "SELECT BODY FROM SMS_ATCH WHERE ID=:ID";
  
  OCIBind   *bindId;
  OCIDefine *defineBody;

  (void) OCIDescriptorAlloc((dvoid *)envhp, (dvoid **)&blobLoc,
                            (ub4)OCI_DTYPE_LOB, (size_t)0, (dvoid **)0);
  
  if (!checkerr (errhp, OCIStmtPrepare(stmthp, errhp, selectSql, 
                                  (ub4)strlen((char *)selectSql),
                                  (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCIBindByPos(stmthp, &bindId, errhp, (ub4) 1,
                                     (dvoid *)&id, (sb4)sizeof(id), 
                                     (ub2) SQLT_BIN,(dvoid *) 0, 
                                     (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                                     (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCIDefineByPos(stmthp, &defineBody, errhp, (ub4) 1,
                                  (dvoid *)&blobLoc, (sb4) 0, 
                                  (ub2) SQLT_BLOB,(dvoid *) 0, 
                                  (ub2 *) 0, (ub2 *) 0, (ub4) OCI_DEFAULT)))
      return false;
  
  /* execute the select and fetch one row */
  if (!checkerr (errhp, OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                                 (CONST OCISnapshot*) 0, (OCISnapshot*) 0,  
                                 (ub4) OCI_DEFAULT)))
      return false;
  
  if (!checkerr (errhp, OCILobOpen(svchp, errhp, blobLoc, OCI_LOB_READONLY)))
      return false;
  if (!checkerr (errhp, OCILobIsOpen(svchp, errhp, blobLoc, &isOpen)))
      return false;
  printf("Lob is %sOpen\n", ((isOpen) ? "":"not "));

  ub4 amount = 0;
  ub4 offset = 1;
  if (!checkerr (errhp, OCILobGetLength (svchp, errhp, blobLoc, &amount )))
      return false;
  
  printf("Reading from LOB ...\nBytes to read: %d\n", amount);
        
  if (!checkerr (errhp, OCILobRead(svchp, errhp, blobLoc, &amount, offset,
                                   (dvoid *)bodyBuffer, sizeof(bodyBuffer), (dvoid *)0,
                                   0, (ub2) 0, (ub1) 0))) return false;

  printf("Bytes read: %d\n", amount);

  if (!checkerr (errhp, OCILobClose(svchp, errhp, blobLoc)))
      return false;
  printf("Lob is Closed !\n");

  /* Free resources held by the locators*/
  (void) OCIDescriptorFree((dvoid *) blobLoc, (ub4) OCI_DTYPE_LOB);
  return (checkerr (errhp, OCITransCommit(svchp, errhp, OCI_DEFAULT)));
}

int main(char *argv, int argc)
{
  /* Declare OCI Handles to be used */
  OCIEnv        *envhp;
  OCIServer     *srvhp; 
  OCISvcCtx     *svchp;
  OCIError      *errhp;
  OCISession    *authp;
  OCIStmt       *stmthp;
  OCILobLocator *Lob_loc;

  /* Create and Initialize an OCI Environment: */
  (void) OCIEnvCreate(&envhp, (ub4)OCI_DEFAULT, (dvoid *)0,
                      (dvoid * (*)(dvoid *, size_t)) 0,
                      (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                      (void (*)(dvoid *, dvoid *))0,
                      (size_t) 0, (dvoid **) 0);

  /* Allocate error handle: */
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, 
                        (size_t) 0, (dvoid **) 0);

  /* Allocate server contexts: */
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
                        (size_t) 0, (dvoid **) 0);

  /* Allocate service context: */
  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
                        (size_t) 0, (dvoid **) 0);

  /* Attach to the Oracle database:  */
  (void) OCIServerAttach(srvhp, errhp, (text *)"DBS_ORCL", strlen("DBS_ORCL"), 0);

  /* Set the server context attribute in the service context: */
  (void) OCIAttrSet ((dvoid *) svchp, OCI_HTYPE_SVCCTX, 
                     (dvoid *)srvhp, (ub4) 0,
                     OCI_ATTR_SERVER, (OCIError *) errhp);

  /* Allocate the session handle: */
  (void) OCIHandleAlloc((dvoid *) envhp, 
                        (dvoid **)&authp, (ub4) OCI_HTYPE_SESSION,
                        (size_t) 0, (dvoid **) 0);
 
  /* Set the username in the session handle:*/
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                    (dvoid *) "smsc", (ub4)4,
                    (ub4) OCI_ATTR_USERNAME, errhp);
  /* Set the password in the session handle: */
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                    (dvoid *) "smsc", (ub4) 4,
                    (ub4) OCI_ATTR_PASSWORD, errhp);

  /* Authenticate and begin the session: */
  checkerr(errhp, OCISessionBegin (svchp,  errhp, authp, OCI_CRED_RDBMS, 
                                   (ub4) OCI_DEFAULT));

  /* Set the session attribute in the service context: */
  (void) OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                    (dvoid *) authp, (ub4) 0,
                   (ub4) OCI_ATTR_SESSION, errhp);

  /* ------- At this point a valid session has been created -----------*/
  printf ("User session created. Connection etablished. \n");

  /* Allocate a statement handle: */
  checkerr(errhp, OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &stmthp,
           OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0));

  /* =============  Sample procedure call begins here ===================*/  

  if (!processLOB(envhp, errhp, svchp, stmthp))
      printf("Process LOB failed !\n");
  else
      printf("Process LOB completed !\n");

  memset(bodyBuffer, 1, sizeof(bodyBuffer));

  if (!writeToDB(envhp, errhp, svchp, stmthp))
      printf("Write failed !\n");
  else
      printf("Write completed !\n");
  
  memset(bodyBuffer, 0, sizeof(bodyBuffer));

  if (!readFromDB(envhp, errhp, svchp, stmthp))
      printf("Read failed !\n");
  else
      printf("Read completed !\n");
  
  for (int i=0; i<sizeof(bodyBuffer); i++)
    if (bodyBuffer[i] != 1){
          printf("Fuck !!!\n");
          return -1;
      }
  
  return 0;
}


