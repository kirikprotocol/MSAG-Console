#include "MainActionFactory.h"

#include "scag/re/actions/ActionIf.h"
#include "scag/re/actions/ActionSet.h"
#include "scag/re/actions/ActionReturn.h"
#include "scag/re/actions/Action.h"
#include "scag/re/actions/misc/ActionLog.h"
#include "scag/re/actions/misc/ActionSend.h"
#include "scag/re/actions/sess/ActionAbort.h"
#include "scag/re/actions/misc/ActionTrafficCheck.h"
#include "scag/re/actions/sess/ActionOperationWait.h"
#include "scag/re/actions/str/ActionMatch.h"
#include "scag/re/actions/bill/BillActionOpen.h"
#include "scag/re/actions/bill/BillActionClose.h"
#include "scag/re/actions/bill/BillActionInfo.h"

#include "scag/re/actions/pers/PersAction.h"
#include "scag/re/actions/pers/BatchAction.h"

#include "scag/re/actions/datetime/DateTimeAction.h"

#include "scag/re/actions/http/HttpCookieAction.h"

#include "scag/re/actions/misc/ActionBinOperations.h"
#include "scag/re/actions/str/ActionSubstr.h"
#include "scag/re/actions/str/ActionConcat.h"
#include "scag/re/actions/str/ActionIndexof.h"
#include "scag/re/actions/bill/ActionBillMoveWait.h"
#include "scag/re/actions/smpp/ActionCloseUssdDialog.h"
#include "scag/re/actions/smpp/ActionRedirect.h"
#include "scag/re/actions/smpp/ActionTLV.h"
#include "scag/re/actions/smpp/ActionReceipt.h"
#include "scag/re/actions/str/ActionLength.h"
#include "scag/re/actions/str/ActionReplace.h"

//#include "scag/SAX2Print.hpp"

namespace scag { namespace re { namespace actions {

using namespace scag::re::actions;
using namespace scag::bill;
using namespace scag::pers;
using namespace scag::transport::http;


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

    if (name=="bill:open") return new BillActionOpen(false);
    if (name=="operation:bill_wait") return new BillActionOpen(true);

    if (name=="bill:close") return new BillActionClose();
    if (name=="bill:info") return new BillActionInfo();

    if (name=="inc") return new ActionInc();
    if (name=="dec") return new ActionDec();

    if (name=="mod") return new ActionMod();
    if (name=="mul") return new ActionMul();
    if (name=="div") return new ActionDiv();

    if (name=="strings:substr") return new ActionSubstr();
    if (name=="strings:concat") return new ActionConcat();
    if (name=="strings:indexof") return new ActionIndexof();
    if (name=="strings:length") return new ActionLength();
    if (name=="strings:replace") return new ActionReplace();

    if (name=="profile:set") return new PersAction(PC_SET);
    if (name=="profile:get") return new PersAction(PC_GET);
    if (name=="profile:del") return new PersAction(PC_DEL);
    if (name=="profile:inc") return new PersAction(PC_INC);
    if (name=="profile:inc-mod") return new PersAction(PC_INC_MOD);
    if (name=="profile:batch") return new BatchAction();

    if (name=="log:debug") return new ActionLog(ActionLog::lgDebug);
    if (name=="log:info") return new ActionLog(ActionLog::lgInfo);
    if (name=="log:warn") return new ActionLog(ActionLog::lgWarning);
    if (name=="log:error") return new ActionLog(ActionLog::lgError);

    if (name=="send:alarm") return new ActionSend(ActionSend::Alarm);
    if (name=="send:warn") return new ActionSend(ActionSend::Warning);
    if (name=="send:info") return new ActionSend(ActionSend::Info);
    if (name=="send:notify") return new ActionSend(ActionSend::Notify);

    if (name=="http:get-cookie") return new CookieAction(false);
    if (name=="http:set-cookie") return new CookieAction(true);

    if (name=="operation:bill_move_wait") return new ActionBillMoveWait();
    if (name=="smpp:close_ussd_dialog") return new ActionCloseUssdDialog();

    if (name=="smpp:redirect") return new ActionRedirect();

    if (name=="smpp:set_tlv") return new ActionTLV(TLV_SET);
    if (name=="smpp:get_tlv") return new ActionTLV(TLV_GET);
    if (name=="smpp:del_tlv") return new ActionTLV(TLV_DEL);    
    if (name=="smpp:exist_tlv") return new ActionTLV(TLV_EXIST);
    if (name=="smpp:receipt") return new ActionReceipt();

    if (name=="datetime:current") return new CurrentDateTimeAction();
    if (name=="datetime:change") return new ChangeDateTimeAction();
    if (name=="datetime:add") return new AddDateTimeAction();
    if (name=="datetime:dec") return new DecDateTimeAction();

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
