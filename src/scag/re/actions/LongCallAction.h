#ifndef __SCAG_RULE_LONG_CALL_ACTION__
#define __SCAG_RULE_LONG_CALL_ACTION__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {


class ActionLongCallInterface
{
protected:
    //void SaveActionPostponeData(ActionContext& context, const void * saveData, int size) = 0;
    //void LoadActionPostponeData(ActionContext& context) = 0;

    virtual bool RunBeforePostpone(ActionContext& context) = 0;
    virtual void ContinueRunning(ActionContext& context) = 0;
};


class LongCallAction : public Action, ActionLongCallInterface
{
    LongCallAction(const LongCallAction&);

public:
    virtual bool run(ActionContext& context)
    {
        if (context.ActionStack.empty()) 
        {
            if (!RunBeforePostpone(context)) return true;

            RuleStatus rs = context.getRuleStatus();
            rs.status = STATUS_LONG_CALL;
            context.setRuleStatus(rs);

            while (!context.ActionStack.empty()) context.ActionStack.pop();

            return false;
        } else
        {
            context.LongCallContext.SetPos(0);
            ContinueRunning(context);
        }

        return true;
    }
    LongCallAction() {};
};



}}}


#endif
