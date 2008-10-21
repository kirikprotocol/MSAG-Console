#include "LongCallContext.h"

namespace scag2 {
namespace lcm {

void LongCallContext::setActionContext( ActionContext* context )
{
    if ( actionContext != context ) delete actionContext;
    actionContext = context;
}


ActionContext* LongCallContext::getActionContext()
{
    return actionContext;
}


void LongCallContext::clear()
{
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
    if(actionContext) delete actionContext;
    clear();
}

} // namespace lcm
} // namespace scag2
