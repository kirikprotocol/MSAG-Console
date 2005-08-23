#include "MainActionFactory.h"

#include "scag/re/actions/ActionIf.h"
#include "scag/re/actions/ActionSet.h"
#include "scag/re/actions/ActionReturn.h"
#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionLog.h"
#include "scag/re/actions/ActionClose.h"
#include "scag/re/actions/ActionTrafficCheck.h"


namespace scag { namespace re { namespace actions {

using namespace scag::re::actions;


Action * MainActionFactory::CreateAction(const std::string& name) const
{
    if (name=="set") return new ActionSet();
    if (name=="if")  return new ActionIf();
    if (name=="return") return new ActionReturn();
    if (name=="traffic:check") return new ActionTrafficCheck();
    if (name=="session:close") return new ActionClose();
    if (name=="log") return new ActionLog();


    Action * action = 0;

    
    for (std::list<const ActionFactory *>::const_iterator it = ChildFactories.begin(); it!=ChildFactories.end();++it)
    {
        action = (*it)->CreateAction(name);
        if (action) break;
    }

    return action;
}

void MainActionFactory::registerChild(const ActionFactory& af)
{
    ChildFactories.push_back(&af);
}



}}}

