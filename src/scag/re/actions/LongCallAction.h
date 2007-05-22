#ifndef __SCAG_RULE_LONG_CALL_ACTION__
#define __SCAG_RULE_LONG_CALL_ACTION__

#include "ActionContext.h"
#include "Action.h"
#include "scag/lcm/LongCallManager.h"

using namespace scag::lcm;

namespace scag { namespace re { namespace actions {

class ComplexActionLongCallHelper
{
protected:
    virtual bool RunActionVector(ActionContext& context, LongCallContext& longCallContext, std::vector<Action *>& actions, Logger * logger)
    {
        int startIndex = 0;

        if (!longCallContext.ActionStack.empty()) longCallContext.ActionStack.pop();

        if (!longCallContext.ActionStack.empty()) 
        {
            startIndex = longCallContext.ActionStack.top().actionIndex;
            smsc_log_debug(logger, "Skeep %d actions", startIndex);

            if(startIndex >= actions.size())
            {
                smsc_log_error(logger, "Cannot continue running actions. Details: action index out of bound startIndex=%d, size=%d", startIndex, actions.size());
                context.clearLongCallContext();
                return true;
            }
        }

        for (int i = startIndex; i < actions.size(); i++)
        {
            if (!actions[i]->run(context)) 
            {
                if (context.getRuleStatus().status == STATUS_LONG_CALL) 
                {
                    smsc_log_debug(logger, "Save to stack %d action number. thenSection=%d", i, actions[i]->isTrueCondition);
                    ActionStackValue sv(i, actions[i]->isTrueCondition);
                    longCallContext.ActionStack.push(sv);
                }

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

            RuleStatus& rs = context.getRuleStatus();
            rs.status = STATUS_LONG_CALL;

            context.clearLongCallContext();
            return false;
        } else
        {
            context.getSCAGCommand().getLongCallContext().ActionStack.pop();
            ContinueRunning(context);
        }

        return true;
    }
    LongCallAction() {};
};

}}}

#endif
