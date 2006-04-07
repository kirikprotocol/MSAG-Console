#include "MainActionFactory.h"

#include "scag/re/actions/ActionIf.h"
#include "scag/re/actions/ActionSet.h"
#include "scag/re/actions/ActionReturn.h"
#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionLog.h"
#include "scag/re/actions/ActionAbort.h"
#include "scag/re/actions/ActionTrafficCheck.h"
#include "scag/re/actions/ActionOperationWait.h"
#include "scag/re/actions/ActionMatch.h"
#include "scag/bill/BillActionOpen.h"
#include "scag/bill/BillActionClose.h"

#include "scag/re/actions/ActionBinOperations.h"


//#include "scag/SAX2Print.hpp"
                 
namespace scag { namespace re { namespace actions {

using namespace scag::re::actions;
using namespace scag::bill;


Action * MainActionFactory::CreateAction(const std::string& name) const
{
    if (name=="set") return new ActionSet();
    if (name=="if")  return new ActionIf();
    if (name=="return") return new ActionReturn();
    if (name=="traffic:check") return new ActionTrafficCheck();
    if (name=="session:abort") return new ActionAbort();
    if (name=="log") return new ActionLog();
    if (name=="operation:wait") return new ActionOperationWait();
    if (name=="match") return new ActionMatch();

    if (name=="bill:open") return new BillActionOpen();
    if (name=="bill:close") return new BillActionClose();

    if (name=="inc") return new ActionInc();
    if (name=="dec") return new ActionDec();

    if (name=="mod") return new ActionMod();
    if (name=="mul") return new ActionMul();
    if (name=="div") return new ActionDiv();

    Action * action = 0;

    for (std::list<const ActionFactory *>::const_iterator it = ChildFactories.begin(); it!=ChildFactories.end();++it)
    {
        action = (*it)->CreateAction(name);
        if (action) break;
    }

    return action;
}

void MainActionFactory::registerChild(const ActionFactory * af) 
{
    if (!af) return;
    ChildFactories.push_back(af);
    smsc_log_info(logger,"Action factory registered");
}



}}}

