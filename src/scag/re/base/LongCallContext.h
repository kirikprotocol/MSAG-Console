/* $Id$ */

#ifndef _SCAG_RE_BASE_LONGCALLCONTEXT_H
#define _SCAG_RE_BASE_LONGCALLCONTEXT_H

#include <stack>

#include "LongCallContextBase.h"
#include "ActionContext2.h"

namespace scag2 {
namespace lcm {

using scag2::re::actions::ActionContext;

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

class LongCallContext : public LongCallContextBase
{
public:
    LongCallContext():actionContext(NULL), actions(NULL) {}

    void setActionContext(ActionContext* context);

    ActionContext* getActionContext();

    void addAction(PostProcessAction* p)
    {
        CHECKMAGTC;
        if(actions)
            actionsTail->next = p;
        else
            actions = p;
        actionsTail = p;
    }

    void runPostProcessActions()
    {
        CHECKMAGTC;
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

private:

public:
    uint32_t systemType;
    std::stack<ActionStackValue> ActionStack;

private:
    ActionContext  *actionContext;
    PostProcessAction *actions, *actionsTail;
};

}
}

#endif
