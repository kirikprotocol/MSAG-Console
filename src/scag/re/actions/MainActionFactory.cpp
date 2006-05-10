#include "MainActionFactory.h"

#include "scag/re/actions/ActionIf.h"
#include "scag/re/actions/ActionSet.h"
#include "scag/re/actions/ActionReturn.h"
#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionLog.h"
#include "scag/re/actions/ActionSend.h"
#include "scag/re/actions/ActionAbort.h"
#include "scag/re/actions/ActionTrafficCheck.h"
#include "scag/re/actions/ActionOperationWait.h"
#include "scag/re/actions/ActionMatch.h"
#include "scag/bill/BillActionOpen.h"
#include "scag/bill/BillActionClose.h"

#include "scag/pers/PersAction.h"

#include "scag/re/actions/ActionBinOperations.h"
#include "scag/re/actions/ActionSubstr.h"
#include "scag/re/actions/ActionConcat.h"
#include "scag/re/actions/ActionIndexof.h"


//#include "scag/SAX2Print.hpp"
                 
namespace scag { namespace re { namespace actions {

using namespace scag::re::actions;
using namespace scag::bill;
using namespace scag::pers;


Action * MainActionFactory::CreateAction(const std::string& name) const
{
    if (name=="set") return new ActionSet();
    if (name=="if")  return new ActionIf();
    if (name=="return") return new ActionReturn();
    if (name=="traffic:check") return new ActionTrafficCheck();
    if (name=="session:abort") return new ActionAbort();
//    if (name=="log") return new ActionLog();
    if (name=="operation:wait") return new ActionOperationWait();
    if (name=="match") return new ActionMatch();

    if (name=="bill:open") return new BillActionOpen();
    if (name=="bill:close") return new BillActionClose();

    if (name=="inc") return new ActionInc();
    if (name=="dec") return new ActionDec();

    if (name=="mod") return new ActionMod();
    if (name=="mul") return new ActionMul();
    if (name=="div") return new ActionDiv();

    if (name=="strings:substr") return new ActionSubstr();
    if (name=="strings:concat") return new ActionConcat();
    if (name=="strings:indexof") return new ActionIndexof();

    if (name=="profile:set") return new PersAction(PC_SET);
    if (name=="profile:get") return new PersAction(PC_GET);
    if (name=="profile:del") return new PersAction(PC_DEL);
    if (name=="profile:inc") return new PersAction(PC_INC);
    if (name=="profile:inc-mod") return new PersAction(PC_INC_MOD);

    if (name=="log:debug") return new ActionLog(ActionLog::lgDebug);
    if (name=="log:info") return new ActionLog(ActionLog::lgInfo);
    if (name=="log:warn") return new ActionLog(ActionLog::lgWarning);
    if (name=="log:error") return new ActionLog(ActionLog::lgError);

    if (name=="send:alarm") return new ActionSend(ActionSend::Alarm);
    if (name=="send:warn") return new ActionSend(ActionSend::Warning);
    if (name=="send:info") return new ActionSend(ActionSend::Info);
    if (name=="send:notify") return new ActionSend(ActionSend::Notify);

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

