#include "MainActionFactory2.h"

#include "ActionIf2.h"
#include "ActionReturn2.h"
#include "ActionSet2.h"
#include "DateTimeAction2.h"
#include "ActionBinOperations2.h"
#include "ActionLog2.h"
#include "ActionSend2.h"
#include "ActionAbort2.h"
#include "ActionSessionContextScope.h"
#include "ActionCloseUssdDialog2.h"
#include "ActionReceipt2.h"
#include "ActionRedirect2.h"
#include "ActionTLV2.h"
#include "ActionConcat2.h"
#include "ActionIndexof2.h"
#include "ActionLength2.h"
#include "ActionMatch2.h"
#include "ActionReplace2.h"
#include "ActionSubstr2.h"

#include "LongCallTestAction.h"

#include "BillActionOpen.h"
#include "BillActionClose.h"
#include "BillActionInfo.h"

/*
#include "scag/re/actions/Action.h"
#include "scag/re/actions/misc/ActionTrafficCheck.h"
#include "scag/re/actions/sess/ActionOperationWait.h"
#include "scag/re/actions/bill/BillActionInfo.h"

#include "scag/re/actions/pers/PersAction.h"
#include "scag/re/actions/pers/BatchAction.h"

#include "scag/re/actions/http/HttpCookieAction.h"

#include "scag/re/actions/bill/ActionBillMoveWait.h"
*/

//#include "scag/SAX2Print.hpp"

namespace scag2 {
namespace re {
namespace actions {

// using namespace scag::re::actions;
// using namespace scag::bill;
// using namespace scag::pers;
// using namespace scag::transport::http;


Action * MainActionFactory::CreateAction( const std::string& name ) const
{
    do { // fake loop

        if (name=="set") return new ActionSet();
        if (name=="return") return new ActionReturn();
        if (name=="match") return new ActionMatch();

        if ( 0 == strncmp(name.c_str(), "strings:", 8) ) {
            if (name=="strings:substr") return new ActionSubstr();
            if (name=="strings:concat") return new ActionConcat();
            if (name=="strings:indexof") return new ActionIndexof();
            if (name=="strings:length") return new ActionLength();
            if (name=="strings:replace") return new ActionReplace();
            break;
        }

        if ( 0 == strncmp(name.c_str(), "log:", 4) ) {
            // if (name=="log") return new ActionLog();
            if (name=="log:debug") return new ActionLog(ActionLog::lgDebug);
            if (name=="log:info") return new ActionLog(ActionLog::lgInfo);
            if (name=="log:warn") return new ActionLog(ActionLog::lgWarning);
            if (name=="log:error") return new ActionLog(ActionLog::lgError);
            break;
        }

        if ( 0 == strncmp(name.c_str(), "session:", 8) ) {
            if (name=="session:abort") 
                return new ActionAbort();
            if (name == "session:new_context") 
                return new ActionSessionContextScope(ActionSessionContextScope::NEW);
            if (name == "session:set_context") 
                return new ActionSessionContextScope(ActionSessionContextScope::SET);
            if (name == "session:del_context") 
                return new ActionSessionContextScope(ActionSessionContextScope::DEL);
            break;
        }

        if (name=="if")  return new ActionIf();

        if (name=="inc") return new ActionInc();
        if (name=="dec") return new ActionDec();
        if (name=="mod") return new ActionMod();
        if (name=="mul") return new ActionMul();
        if (name=="div") return new ActionDiv();

        if ( 0 == strncmp(name.c_str(),"send:", 5) ) {
            if (name=="send:alarm") return new ActionSend(ActionSend::Alarm);
            if (name=="send:warn") return new ActionSend(ActionSend::Warning);
            if (name=="send:info") return new ActionSend(ActionSend::Info);
            if (name=="send:notify") return new ActionSend(ActionSend::Notify);
            break;
        }

        if ( 0 == strncmp(name.c_str(),"smpp:", 5) ) {
            if (name=="smpp:close_ussd_dialog") return new ActionCloseUssdDialog();
            if (name=="smpp:redirect") return new ActionRedirect();
            if (name=="smpp:set_tlv") return new ActionTLV(TLV_SET);
            if (name=="smpp:get_tlv") return new ActionTLV(TLV_GET);
            if (name=="smpp:del_tlv") return new ActionTLV(TLV_DEL);    
            if (name=="smpp:exist_tlv") return new ActionTLV(TLV_EXIST);
            if (name=="smpp:receipt") return new ActionReceipt();
            break;
        }

        if ( 0 == strncmp(name.c_str(),"datetime:",9) ) {
            if (name=="datetime:current") return new CurrentDateTimeAction();
            if (name=="datetime:change") return new ChangeDateTimeAction();
            if (name=="datetime:add") return new AddDateTimeAction();
            if (name=="datetime:dec") return new DecDateTimeAction();
            break;
        }


        if ( name == "longcalltest" ) return new LongCallTestAction();

        if ( 0 == strncmp(name.c_str(), "bill:", 5 ) ) {
            if ( name == "bill:open" ) return new BillActionOpen();
            if ( name == "bill:close" ) return new BillActionClose();
            if ( name == "bill:info" ) return new BillActionInfo();
            break;
        }

    // FIXME: impl
    /* 
    if (name=="traffic:check") return new ActionTrafficCheck();
    if (name=="operation:wait") return new ActionOperationWait();

    if (name=="bill:open") return new BillActionOpen(false);
    if (name=="operation:bill_wait") return new BillActionOpen(true);

    if (name=="bill:close") return new BillActionClose();
    if (name=="bill:info") return new BillActionInfo();

    if (name=="profile:set") return new PersAction(PC_SET);
    if (name=="profile:get") return new PersAction(PC_GET);
    if (name=="profile:del") return new PersAction(PC_DEL);
    if (name=="profile:inc") return new PersAction(PC_INC);
    if (name=="profile:inc-mod") return new PersAction(PC_INC_MOD);
    if (name=="profile:batch") return new BatchAction();

    if (name=="http:get-cookie") return new CookieAction(false);
    if (name=="http:set-cookie") return new CookieAction(true);

    if (name=="operation:bill_move_wait") return new ActionBillMoveWait();
     */

    } while ( false ); // fake loop

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
