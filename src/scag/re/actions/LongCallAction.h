#ifndef __SCAG_RULE_LONG_CALL_ACTION__
#define __SCAG_RULE_LONG_CALL_ACTION__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {


class ActionLongCallInterface
{
protected:
    void RunBeforePostpone(ActionContext& context);
    //void SaveActionPostponeData(ActionContext& context, const void * saveData, int size) = 0;
    //void LoadActionPostponeData(ActionContext& context) = 0;
    void RunAfterPostpone(ActionContext& context) = 0;
};


class LongCallAction : public Action, ActionLongCallInterface
{
    ActionLongCallInterface(const ActionLongCallInterface&);

public:
    virtual bool run(ActionContext& context)
    {
        if (context.ActionStack.empty()) 
        {
            RunBeforePostpone(context);
            RuleStatus rs = context.getRuleStatus();
            rs.status = STATUS_LONG_CALL;
            context.setRuleStatus(rs);

            return false;
        } else
            RunAfterPostpone(ActionContext& context);

        return true;
    }
};



}}}


#endif
