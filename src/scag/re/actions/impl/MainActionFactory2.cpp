#include "MainActionFactory2.h"

#include "ActionIf2.h"
#include "ActionReturn2.h"
#include "ActionSet2.h"
#include "ActionDel.h"
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
#include "HttpCookieAction.h"

// #include "LongCallTestAction.h"

#include "BillActionOpen.h"
#include "BillActionClose.h"
#include "BillActionCloseTransit.h"
#include "BillActionInfo.h"
#include "BillActionMkid.h"
#include "BillActionCheck.h"
#include "BillActionTransfer.h"
#include "BillActionInfoTransit.h"
// #include "BillActionKeywords.h"
#include "BillActionTariff.h"

#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
// #include "scag/pvss/api/packets/BatchCommand.h"

#include "ActionParsePairs.h"
#include "ActionFormatPairs.h"
#include "ActionSplitPairs.h"

// counters
#include "ActionCounter.h"

namespace scag2 {
namespace re {
namespace actions {

Action * MainActionFactory::CreateAction( const std::string& name ) const
{
    do { // fake loop

        if (name=="set") return new ActionSet();
        if (name=="del") return new ActionDel();
        if (name=="return") return new ActionReturn();
        if (name=="match") return new ActionMatch();

        if ( 0 == strncmp(name.c_str(), "strings:", 8) ) {
            if (name=="strings:substr") return new ActionSubstr();
            if (name=="strings:concat") return new ActionConcat();
            if (name=="strings:indexof") return new ActionIndexof();
            if (name=="strings:length") return new ActionLength();
            if (name=="strings:replace") return new ActionReplace();

            if (name=="strings:parse_pairs") return new ActionParsePairs();
            if (name=="strings:split_pairs") return new ActionSplitPairs();
            if (name=="strings:format_pairs") return new ActionFormatPairs();
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
            if ( name == "bill:mkid" ) return new BillActionMkid();
            if ( name == "bill:check" ) return new BillActionCheck();
            if ( name == "bill:transfer" ) return new BillActionTransfer();
            if ( name == "bill:open" ) return new BillActionOpen(false);
            if ( name == "bill:open-transit" ) return new BillActionOpen(true);
            if ( name == "bill:close" ) return new BillActionClose();
            if ( name == "bill:close-transit" ) return new BillActionCloseTransit();
            if ( name == "bill:info" ) return new BillActionInfo();
            if ( name == "bill:info-transit" ) return new BillActionInfoTransit();
            // if ( name == "bill:set_keywords" ) return new BillActionSetKeywords();
            // if ( name == "bill:add_keywords" ) return new BillActionAddKeywords();
            if ( name == "bill:tariff" ) return new BillActionTariff();
            break;
        }
        if ( 0 == strncmp(name.c_str(), "profile:", 8 ) ) {
            if (name=="profile:set") return new PersAction(new pvss::SetCommand,"profile:set");
            if (name=="profile:get") return new PersAction(new pvss::GetCommand,"profile:get");
            if (name=="profile:del") return new PersAction(new pvss::DelCommand,"profile:del");
            if (name=="profile:inc") return new PersAction(new pvss::IncCommand,"profile:inc");
            if (name=="profile:inc-mod") return new PersAction(new pvss::IncModCommand,"profile:inc-mod");
            if (name=="profile:batch") return new BatchAction();
            break;
        }
        if ( 0 == strncmp(name.c_str(), "batch:", 6 ) ) {
            if (name=="batch:set") return new PersActionCommand(new pvss::SetCommand,"batch:set");
            if (name=="batch:get") return new PersActionCommand(new pvss::GetCommand,"batch:get");
            if (name=="batch:del") return new PersActionCommand(new pvss::DelCommand,"batch:del");
            if (name=="batch:inc") return new PersActionCommand(new pvss::IncCommand,"batch:inc");
            if (name=="batch:inc-mod") return new PersActionCommand(new pvss::IncModCommand,"batch:inc-mod");
            break;
        }
        if ( 0 == strncmp(name.c_str(), "stat:", 5 ) ) {
            if (name == "stat:add_keywords") return new AddKeywordsAction();
            if (name == "stat:set_keywords") return new SetKeywordsAction();
            if (name == "stat:get_keywords") return new GetKeywordsAction();
            break;
        }

        if (0 == strncmp(name.c_str(),"counter:",8)) {
            if (name == "counter:create") return new ActionCounterCreate();
            if (name == "counter:reset") return new ActionCounterReset();
            if (name == "counter:inc") return new ActionCounterInc();
            if (name == "counter:get") return new ActionCounterGet();
            break;
        }

        if (name=="http:get-cookie") return new CookieAction(CookieAction::GET);
        if (name=="http:set-cookie") return new CookieAction(CookieAction::SET);
        if (name=="http:det-cookie") return new CookieAction(CookieAction::DEL);

    /* 
    if (name=="traffic:check") return new ActionTrafficCheck();

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
