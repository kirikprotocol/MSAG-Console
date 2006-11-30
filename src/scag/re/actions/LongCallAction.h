#ifndef __SCAG_RULE_LONG_CALL_ACTION__
#define __SCAG_RULE_LONG_CALL_ACTION__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {

class ComplexActionLongCallHelper
{
protected:
    virtual bool RunActionVector(ActionContext& context, LongCallContext& longCallContext, std::vector<Action *>& actions, Logger * logger)
    {
        int startIndex = 0;

        if (!longCallContext.ActionStack.empty()) 
        {
            startIndex = longCallContext.ActionStack.top().actionIndex;
            if (startIndex >= actions.size())
            {
                smsc_log_error(logger, "Cannot continue running actions. Details: action index out of bound");
                context.clearLongCallContext();
                return true;
            }
            longCallContext.ActionStack.pop();
        }

        for (int i = startIndex; i < actions.size(); i++)
        {
            if (!actions[i]->run(context)) 
            {
                ActionStackValue sv(i, false);
                smsc_log_debug(logger,"Run Action 'if' !!!1");
                longCallContext.ActionStack.push(sv);
                smsc_log_debug(logger,"Run Action 'if' !!!2");
                return false;
            }
        }
        return true;    
    }
};

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
        if (context.getSCAGCommand().getLongCallContext().ActionStack.empty()) 
        {
            if (!RunBeforePostpone(context)) return true;

            RuleStatus rs = context.getRuleStatus();
            rs.status = STATUS_LONG_CALL;
            context.setRuleStatus(rs);

            context.clearLongCallContext();

            return false;
        } else
        {
            //context.getBuffer().SetPos(0);
            ContinueRunning(context);
        }

        return true;
    }
    LongCallAction() {};
};



}}}


#endif
