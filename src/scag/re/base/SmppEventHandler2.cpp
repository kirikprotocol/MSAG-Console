#include "SmppEventHandler2.h"
#include "SmppAdapter2.h"
#include "ActionContext2.h"
#include "CommandBridge.h"
#include "RuleEngine2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace re {

using namespace smpp;

void SmppEventHandler::process( SCAGCommand& command, Session& session, RuleStatus& rs, CommandProperty& cp, util::HRTiming* inhrt )
{
    // smsc_log_debug(logger, "Process EventHandler...");
    util::HRTiming hrt(inhrt);

    SmppCommand& smppcommand = static_cast<SmppCommand&>(command);
    SmppCommandAdapter _command(smppcommand);

    ActionContext* actionContext = 0;
    if(session.getLongCallContext().continueExec) {
    	actionContext = session.getLongCallContext().getActionContext();
    	if (actionContext) {
	        actionContext->resetContext(&(RuleEngine::Instance().getConstants()), 
	                    				&session, &_command, &cp, &rs);
    	} else {
	        smsc_log_error(logger, "EventHandler cannot get actionContext to continue");
    	    rs.result = smsc::system::Status::SMDELIFERYFAILURE;
    	    rs.status = STATUS_FAILED;
    	    return;
    	}
    } else {
    	actionContext = new ActionContext(&(RuleEngine::Instance().getConstants()), 
                    					  &session, &_command, &cp, &rs);
    	session.getLongCallContext().setActionContext(actionContext);
    }
    hrt.mark("ev.prep");
    
    try {
        RunActions(*actionContext);
    } catch (SCAGException& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    } catch (std::exception& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    }
    // hrt.mark("ev.run");

    if (!session.getLongCallContext().continueExec) {
      const std::string* kw = session.getCurrentOperation() ? session.getCurrentOperation()->getKeywords() : 0;
      cp.keywords = kw ? *kw : "";
    }

    /*
    if ( smppcommand.get_status() > 0) {
        rs.result = smppcommand.get_status();
        rs.status = STATUS_FAILED;
        smsc_log_debug( logger, "Command status=%d(%x) overrides RE status", rs.result, rs.result );
    }
     */
    hrt.mark("ev.post");
    return;
}


int SmppEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "submit_sm")             return EH_SUBMIT_SM;
    if (str == "submit_sm_resp")        return EH_SUBMIT_SM_RESP;
    if (str == "deliver_sm")            return EH_DELIVER_SM;
    if (str == "deliver_sm_resp")       return EH_DELIVER_SM_RESP;
    // if (str == "receipt")               return EH_RECEIPT;
    if (str == "data_sm")               return EH_DATA_SM;
    if (str == "data_sm_resp")          return EH_DATA_SM_RESP;
    return UNKNOWN; 
}

}}
