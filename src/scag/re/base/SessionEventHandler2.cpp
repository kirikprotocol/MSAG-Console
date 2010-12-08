#include "SessionEventHandler2.h"
#include "SessionAdapter2.h"
#include "RuleEngine2.h"
#include "CommandBridge.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/transport/CommandOperation.h"

namespace scag2 {
namespace re {

using namespace session;

void SessionEventHandler::process(SCAGCommand& command, Session& session, RuleStatus& rs, CommandProperty& cp, util::HRTiming* ) {
    throw SCAGException("session event handler should not be invoked directly");
    // _process(session, rs);
}

void SessionEventHandler::_process(Session& session,
                                   RuleStatus& rs,
                                   const RuleKey& key )
{
    // smsc_log_debug(logger, "Process SessionEventHandler...");

    bill::infrastruct::Infrastructure& istr = 
        bill::BillingManager::Instance().getInfrastructure();

    Address abonentAddr( session.sessionKey().toString().c_str() );

    if ( ! session.hasRuleKey( key.serviceId, key.transport ) ) {
        throw SCAGException("SessionEventHandler: rule key is not found in session=%p", &session);
    }
    int providerId = istr.GetProviderID(key.serviceId);
    if (providerId == 0) {
        throw SCAGException("SessionEventHandler: Cannot find ProviderID for ServiceID=%d", key.serviceId);
    }
    int operatorId = istr.GetOperatorID(abonentAddr);
    // int hi = propertyObject.HandlerId;
    if (operatorId == 0) {
        throw SCAGException("SessionEventHandler: Cannot find OperatorID for %s abonent", abonentAddr.toString().c_str());
    }
    
    SessionAdapter _command(session);
    Property routeId;
    routeId.setInt(0);
    CommandProperty commandProperty(NULL, 0, abonentAddr, providerId, operatorId, key.serviceId,
                                    0, transport::CO_NA, routeId, propertyObject.HandlerId);
    ActionContext* actionContext = 0;
    if(session.getLongCallContext().continueExec) {
	actionContext = session.getLongCallContext().getActionContext();
	if (actionContext) {
	    actionContext->resetContext(&(RuleEngine::Instance().getConstants()),
	                                &session, &_command, &commandProperty, &rs);
	} else {
	    smsc_log_error(logger, "EventHandler cannot get actionContext to continue");
    	    rs.status = STATUS_FAILED;
    	    return;
	}
    } else {
	actionContext = new ActionContext(&(RuleEngine::Instance().getConstants()),
	                        	  &session, &_command, &commandProperty, &rs);
	session.getLongCallContext().setActionContext(actionContext);
    }

    try
    {
        RunActions(*actionContext);
    } 
    catch (SCAGException& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    } 
    catch (std::exception& e) {
        smsc_log_error(logger, "EventHandler: error in actions processing. Details: %s", e.what());
        rs.status = STATUS_FAILED;
    }
    return;
}

int SessionEventHandler::StrToHandlerId(const std::string& str)
{
    if (str == "service_init")      return EH_SESSION_INIT;
    if (str == "service_destroy")   return EH_SESSION_DESTROY;
    return UNKNOWN; 
}

}}
