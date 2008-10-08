#include "MainActionFactory2.h"

#include "ActionIf2.h"
#include "ActionReturn2.h"
#include "ActionSet2.h"
#include "DateTimeAction2.h"
#include "ActionBinOperations2.h"
#include "ActionLog2.h"
#include "ActionSend2.h"
#include "ActionSessionContextScope.h"
#include "ActionSessionDestroyService.h"
#include "ActionSessionWait.h"
#include "ActionOperationMakePersistent.h"
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

#include "BatchAction2.h"
#include "PersAction2.h"
#include "StatAction.h"

// #include "LongCallTestAction.h"

#include "BillActionOpen.h"
#include "BillActionClose.h"
#include "BillActionInfo.h"
#include "BillActionKeywords.h"

namespace scag2 {
namespace re {
namespace actions {

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
            if (name=="log:debug") return new ActionLog(ActionLog::lgDebug);
            if (name=="log:info") return new ActionLog(ActionLog::lgInfo);
            if (name=="log:warn") return new ActionLog(ActionLog::lgWarning);
            if (name=="log:error") return new ActionLog(ActionLog::lgError);
            break;
        }

        if ( 0 == strncmp(name.c_str(), "session:", 8) ) {
            if (name=="session:wait")
                return new ActionSessionWait();
            if (name=="session:destroy_service")
                return new ActionSessionDestroyService();
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


        if ( name == "operation:make_persistent" ) {
            return new ActionOperationMakePersistent();
        }

        // if ( name == "longcalltest" ) return new LongCallTestAction();

        if ( 0 == strncmp(name.c_str(), "bill:", 5 ) ) {
            if ( name == "bill:open" ) return new BillActionOpen();
            if ( name == "bill:close" ) return new BillActionClose();
            if ( name == "bill:info" ) return new BillActionInfo();
            if ( name == "bill:set_keywords" ) return new BillActionSetKeywords();
            if ( name == "bill:add_keywords" ) return new BillActionAddKeywords();
            break;
        }
        if ( 0 == strncmp(name.c_str(), "profile:", 8 ) ) {
            if (name=="profile:set") return new PersAction(PC_SET);
            if (name=="profile:get") return new PersAction(PC_GET);
            if (name=="profile:del") return new PersAction(PC_DEL);
            if (name=="profile:inc") return new PersAction(PC_INC);
            if (name=="profile:inc-mod") return new PersAction(PC_INC_MOD);
            if (name=="profile:batch") return new BatchAction();
            break;
        }
        if ( 0 == strncmp(name.c_str(), "batch:", 6 ) ) {
            if (name=="batch:set") return new PersActionCommand(PC_SET);
            if (name=="batch:get") return new PersActionCommand(PC_GET);
            if (name=="batch:del") return new PersActionCommand(PC_DEL);
            if (name=="batch:inc") return new PersActionCommand(PC_INC);
            if (name=="batch:inc-mod") return new PersActionCommand(PC_INC_MOD);
            break;
        }
        if ( 0 == strncmp(name.c_str(), "stat:", 5 ) ) {
            if (name == "stat:add_keywords") return new AddKeywordsAction();
            if (name == "stat:set_keywords") return new SetKeywordsAction();
            if (name == "stat:get_keywords") return new GetKeywordsAction();
            break;
        }

    // FIXME: implement other actions
    /* 
    if (name=="traffic:check") return new ActionTrafficCheck();
    if (name=="http:get-cookie") return new CookieAction(false);
    if (name=="http:set-cookie") return new CookieAction(true);

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
