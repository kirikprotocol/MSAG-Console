#ifndef _SCAG_RE_BASE_LONGCALLCONTEXTBASE_H
#define _SCAG_RE_BASE_LONGCALLCONTEXTBASE_H

#include <sys/types.h>
#include <stack>
#include <string>
#include "util/int.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace lcm {

enum LongCallCommandId 
{
    PERS_GET = 1,
    PERS_SET,
    PERS_DEL,
    PERS_INC,
    PERS_INC_MOD,
    PERS_BATCH,
    BILL_OPEN,
    BILL_COMMIT,
    BILL_ROLLBACK,
    BILL_CHECK,
    BILL_TRANSFER,
    BILL_INFO
};

class LongCallParams
{
private:
    DECLMAGTC(LongCallParams);
public:
    std::string exception;
    virtual ~LongCallParams() {}
};

class LongCallInitiator;

class LongCallContextBase
{
public:
    LongCallContextBase(): initiator(NULL), next(NULL), stateMachineContext(NULL), continueExec(false), params(NULL) {}

    LongCallParams* getParams() {
        CHECKMAGTC;
        return params;
    }

    void setParams(LongCallParams* p)
    {
        CHECKMAGTC;
        if (params != p) delete params;
        params = p;
    }

    void freeParams()
    {
        CHECKMAGTC;
        if(params) delete params;
        params = NULL;
    }

    virtual ~LongCallContextBase()
    {
        CHECKMAGTC;
        if (params) {
          delete params;
        }
    }


protected:
    DECLMAGTC(LongCallContextBase);
public:
    uint32_t callCommandId;
    LongCallInitiator *initiator;
    LongCallContextBase *next;
    void *stateMachineContext;
    bool continueExec;

private:
    LongCallParams *params;
};

class LongCallInitiator
{
public:
    virtual ~LongCallInitiator() {}
    virtual void continueExecution(LongCallContextBase* context, bool dropped) = 0;
};

}
}

#endif
