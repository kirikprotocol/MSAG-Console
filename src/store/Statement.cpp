#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Statement.h"

namespace smsc { namespace store 
{
/* ----------------------------- Statetment -------------------------- */
//using smsc::util::Logger;
//log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");

Statement::Statement() : envhp(0L), svchp(0L), stmt(0L) 
{

}

Statement::~Statement()
{
    (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
}

sword Statement::create(OCIEnv* envhp, OCISvcCtx* svchp,
                        OCIError* errhp, text* sql)
{
    this->envhp = envhp; this->svchp = svchp;
    sword status = OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmt,
                                  OCI_HTYPE_STMT, 0, (dvoid **)0);
    return ((status == OCI_SUCCESS) ? 
                OCIStmtPrepare(stmt, errhp, sql, (ub4) strlen((char *)sql),
                               (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT) 
                : status);
}

sword Statement::bind(ub4 pos, ub2 type, 
                      dvoid* placeholder, sb4 size, OCIError* errhp)
{
    OCIBind *bind;
    sword status = OCIBindByPos(stmt, &bind, errhp, pos, 
                                placeholder, size, type, 
                                (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT);
    if (status == OCI_SUCCESS) binds.Push(bind);
    return status;
}

sword Statement::bind(CONST text* name, sb4 name_len, ub2 type,
                      dvoid* placeholder, sb4 size, OCIError* errhp)
{
    OCIBind *bind;
    sword status = OCIBindByName(stmt, &bind, errhp, name, name_len,
                                 placeholder, size, type,
                                 (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                 (ub4) 0, (ub4 *) 0, (ub4) OCI_DEFAULT);
    if (status == OCI_SUCCESS) binds.Push(bind);
    return status;
}

sword Statement::define(ub4 pos, ub2 type, 
                        dvoid* placeholder, sb4 size, OCIError* errhp)
{
    OCIDefine*  define;
    sword status = OCIDefineByPos(stmt, &define, errhp, pos, 
                                  placeholder, size, type, (dvoid *) 0,
                                  (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT);
    if (status == OCI_SUCCESS) defines.Push(define);
    return status;
}

sword Statement::execute(OCIError* errhp, ub4 mode, 
                         ub4 iters, ub4 rowoff)
{
    return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          mode);
}

sword Statement::fetch(OCIError* errhp)
{
    return OCI_SUCCESS; // Need to implement it later !
}

}}

