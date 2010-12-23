#include "LongCallContext.h"

namespace scag2 {
namespace lcm {

void LongCallContext::setActionContext( ActionContext* context )
{
    CHECKMAGTC;
    if ( actionContext != context ) delete actionContext;
    actionContext = context;
}


ActionContext* LongCallContext::getActionContext()
{
    CHECKMAGTC;
    return actionContext;
}


void LongCallContext::clear()
{
    CHECKMAGTC;
    continueExec =false;
    while(actions)
    {
        PostProcessAction* c = actions;
        actions = actions->next;
        delete c;
    }
    actions = NULL;
}

LongCallContext::~LongCallContext()
{
    CHECKMAGTC;
    if(actionContext) delete actionContext;
    clear();
}

} // namespace lcm
} // namespace scag2
