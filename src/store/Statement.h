#ifndef STATEMENT_DECLARATIONS
#define STATEMENT_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <core/buffers/Array.hpp>
//#include <util/Logger.h>

namespace smsc { namespace store 
{
    //using smsc::util::Logger;
    using smsc::core::buffers::Array;

    class Statement
    {
    protected:

        //log4cpp::Category    &log;

        Array<OCIBind *>    binds;
        Array<OCIDefine *>  defines;

        OCIEnv      *envhp;
        OCISvcCtx   *svchp;
        OCIStmt     *stmt;
        
    public:
        
        Statement();
        ~Statement();

        sword create(OCIEnv* envhp, OCISvcCtx* svchp,
                     OCIError* errhp, text* sql);

        sword bind(ub4 pos, ub2 type, 
                   dvoid* placeholder, sb4 size, OCIError* errhp);
        sword bind(CONST text* name, sb4 name_len, ub2 type,
                   dvoid* placeholder, sb4 size, OCIError* errhp); 
        
        sword define(ub4 pos, ub2 type, 
                     dvoid* placeholder, sb4 size, OCIError* errhp);

        sword execute(OCIError* errhp, ub4 mode=OCI_DEFAULT, 
                      ub4 iters=1, ub4 rowoff=0);
        sword fetch(OCIError* errhp);

    };

}}

#endif
