/* $Id$ */

#ifndef _SCAG_RE_BASE_LONGCALLCONTEXT_H
#define _SCAG_RE_BASE_LONGCALLCONTEXT_H

#include "ActionContext2.h"

#include <stack>

namespace scag2 {
namespace lcm {

using scag2::re::actions::ActionContext;

/*
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
    BILL_ROLLBACK
};
 */

class LongCallParams
{
public:
    std::string exception;
    virtual ~LongCallParams() {};
};

struct ActionStackValue
{
    int actionIndex;
    bool thenSection;
    ActionStackValue(int index, bool flag) : actionIndex(index), thenSection(flag) {}
};


class PostProcessAction
{
public:
    PostProcessAction* next;
    PostProcessAction() : next(NULL) {};
    virtual void run() = 0;
    virtual ~PostProcessAction() {};
};

class LongCallInitiator;

class LongCallContext
{
public:
    LongCallContext():
    stateMachineContext(NULL),
    initiator(NULL),
    next(NULL),
    continueExec(false),
    params(NULL),
    actionContext(NULL),
    actions(NULL)
    {}

    LongCallParams* getParams() { return params; }
    void setParams(LongCallParams* p)
    {
        if (params != p) delete params;
        params = p;
    }
    void freeParams()
    {
        if(params) delete params;
        params = NULL;
    }

    void setActionContext(ActionContext* context);
    ActionContext* getActionContext();

    void addAction(PostProcessAction* p)
    {
        if(actions)
            actionsTail->next = p;
        else
            actions = p;
        actionsTail = p;
    }

    void runPostProcessActions()
    {
        PostProcessAction* p;
        while(actions)
        {
            actions->run();
            p = actions->next;
            delete actions;
            actions=p;
        }
        actionsTail=actions;
    }

    void clear();

    ~LongCallContext();


public:
    uint32_t systemType, callCommandId;
    void *stateMachineContext;
    LongCallInitiator *initiator;
    LongCallContext *next;
    bool continueExec;
    std::stack<ActionStackValue> ActionStack;

private:
    LongCallParams *params;
    ActionContext  *actionContext;
    PostProcessAction *actions, *actionsTail;

};


class LongCallInitiator
{
public:
    virtual void continueExecution(LongCallContext* context, bool dropped) = 0;
};

}
}

#endif
